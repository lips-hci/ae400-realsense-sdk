// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

#ifdef RS2_USE_WINUSB_UVC_BACKEND

#if (_MSC_FULL_VER < 180031101)
    #error At least Visual Studio 2013 Update 4 is required to compile this backend
#endif

#include "../types.h"
#include "win7-helpers.h"

#include <Cfgmgr32.h>
#include <usbioctl.h>
#include <SetupAPI.h>
#include <comdef.h>
#include <atlstr.h>
#include <Windows.h>
#include <SetupAPI.h>
#include <string>
#include <regex>
#include <Sddl.h>

#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "setupapi.lib")

#include <initguid.h>

#ifndef WITH_TRACKING
DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, \
    0xC0, 0x4F, 0xB9, 0x51, 0xED);
#endif

DEFINE_GUID(GUID_DEVINTERFACE_IMAGE, 0xe659c3ec, 0xbf3c, 0x48a5, 0x81, 0x92, 0x30, 0x73, 0xe8, 0x22, 0xd7, 0xcd);
DEFINE_GUID(GUID_DEVINTERFACE_CAMERA, 0x50537bc3, 0x2919, 0x452d, 0x88, 0xa9, 0xb1, 0x3b, 0xbf, 0x7d, 0x24, 0x59);

#define CREATE_MUTEX_RETRY_NUM  (5)

namespace librealsense
{
    namespace platform
    {
        std::string hr_to_string(HRESULT hr)
        {
            _com_error err(hr);
            std::wstring errorMessage = (err.ErrorMessage()) ? err.ErrorMessage() : L"";
            std::stringstream ss;
            ss << "HResult 0x" << std::hex << hr << ": \"" << std::string(errorMessage.begin(), errorMessage.end()) << "\"";
            return ss.str();
        }

        typedef ULONG(__stdcall* fnRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);


        bool is_win10_redstone2()
        {
            RTL_OSVERSIONINFOEXW verInfo = { 0 };
            verInfo.dwOSVersionInfoSize = sizeof(verInfo);
            static auto RtlGetVersion = reinterpret_cast<fnRtlGetVersion>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion"));
            if (RtlGetVersion != nullptr && RtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&verInfo)) == 0)
            {
                return verInfo.dwMajorVersion >= 0x0A && verInfo.dwBuildNumber >= 15063;
            }
            else
                return false;
        }

        bool check(const char * call, HRESULT hr, bool to_throw)
        {
            if (FAILED(hr))
            {
                std::string descr = to_string() << call << " returned: " << hr_to_string(hr);
                if (to_throw)
                    throw windows_backend_exception(descr);
                else
                    LOG_INFO(descr);

                return false;
            }
            return true;
        }

        std::string win_to_utf(const WCHAR * s)
        {
            auto len = WideCharToMultiByte(CP_UTF8, 0, s, -1, nullptr, 0, nullptr, nullptr);
            if(len == 0)
                throw std::runtime_error(to_string() << "WideCharToMultiByte(...) returned 0 and GetLastError() is " << GetLastError());

            std::string buffer(len-1, ' ');
            len = WideCharToMultiByte(CP_UTF8, 0, s, -1, &buffer[0], static_cast<int>(buffer.size())+1, nullptr, nullptr);
            if(len == 0)
                throw std::runtime_error(to_string() << "WideCharToMultiByte(...) returned 0 and GetLastError() is " << GetLastError());

            return buffer;
        }

        std::vector<std::string> tokenize(std::string string, char separator)
        {
            std::vector<std::string> tokens;
            std::string::size_type i1 = 0;
            while(true)
            {
                auto i2 = string.find(separator, i1);
                if(i2 == std::string::npos)
                {
                    tokens.push_back(string.substr(i1));
                    return tokens;
                }
                tokens.push_back(string.substr(i1, i2-i1));
                i1 = i2+1;
            }
        }

        bool parse_usb_path(uint16_t & vid, uint16_t & pid, uint16_t & mi, std::string & unique_id, std::string & device_guid, const std::string & path)
        {
            auto name = path;
            std::transform(begin(name), end(name), begin(name), ::tolower);
            auto tokens = tokenize(name, '#');
            if(tokens.size() < 1 || (tokens[0] != R"(\\?\usb)" && tokens[0] != R"(\\?\hid)")) return false; // Not a USB device
            if(tokens.size() < 3)
            {
                LOG_ERROR("malformed usb device path: " << name);
                return false;
            }

            auto ids = tokenize(tokens[1], '&');
            if(ids[0].size() != 8 || ids[0].substr(0,4) != "vid_" || !(std::istringstream(ids[0].substr(4,4)) >> std::hex >> vid))
            {
                LOG_ERROR("malformed vid string: " << tokens[1]);
                return false;
            }

            if(ids[1].size() != 8 || ids[1].substr(0,4) != "pid_" || !(std::istringstream(ids[1].substr(4,4)) >> std::hex >> pid))
            {
                LOG_ERROR("malformed pid string: " << tokens[1]);
                return false;
            }

            if(ids.size() > 2 && (ids[2].size() != 5 || ids[2].substr(0,3) != "mi_" || !(std::istringstream(ids[2].substr(3,2)) >> mi)))
            {
                LOG_ERROR("malformed mi string: " << tokens[1]);
                return false;
            }

            ids = tokenize(tokens[2], '&');
            if(ids.size() == 0)
            {
                LOG_ERROR("malformed id string: " << tokens[2]);
                return false;
            }

            if (ids.size() > 2)
                unique_id = ids[1];
            else
                unique_id = "";

            device_guid = tokens[3];

            return true;
        }

        bool parse_usb_path_from_device_id(uint16_t & vid, uint16_t & pid, uint16_t & mi, std::string & unique_id, const std::string & device_id)
        {
            auto name = device_id;
            std::transform(begin(name), end(name), begin(name), ::tolower);
            auto tokens = tokenize(name, '\\');
            if (tokens.size() < 1 || tokens[0] != R"(usb)") return false; // Not a USB device

            auto ids = tokenize(tokens[1], '&');
            if (ids[0].size() != 8 || ids[0].substr(0, 4) != "vid_" || !(std::istringstream(ids[0].substr(4, 4)) >> std::hex >> vid))
            {
                LOG_ERROR("malformed vid string: " << tokens[1]);
                return false;
            }

            if (ids[1].size() != 8 || ids[1].substr(0, 4) != "pid_" || !(std::istringstream(ids[1].substr(4, 4)) >> std::hex >> pid))
            {
                LOG_ERROR("malformed pid string: " << tokens[1]);
                return false;
            }

            if (ids[2].size() != 5 || ids[2].substr(0, 3) != "mi_" || !(std::istringstream(ids[2].substr(3, 2)) >> mi))
            {
                LOG_ERROR("malformed mi string: " << tokens[1]);
                return false;
            }

            ids = tokenize(tokens[2], '&');
            if (ids.size() < 2)
            {
                LOG_ERROR("malformed id string: " << tokens[2]);
                return false;
            }
            unique_id = ids[1];
            return true;
        }

        bool handle_node(const std::wstring & targetKey, HANDLE h, ULONG index)
        {
            USB_NODE_CONNECTION_DRIVERKEY_NAME key;
            key.ConnectionIndex = index;

            if (!DeviceIoControl(h, IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME, &key, sizeof(key), &key, sizeof(key), nullptr, nullptr))
            {
                return false;
            }

            if (key.ActualLength < sizeof(key)) return false;

            auto alloc = std::malloc(key.ActualLength);
            if (!alloc)
                throw std::bad_alloc();

            auto pKey = std::shared_ptr<USB_NODE_CONNECTION_DRIVERKEY_NAME>(reinterpret_cast<USB_NODE_CONNECTION_DRIVERKEY_NAME *>(alloc), std::free);

            pKey->ConnectionIndex = index;
            if (DeviceIoControl(h, IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME, pKey.get(), key.ActualLength, pKey.get(), key.ActualLength, nullptr, nullptr))
            {
                //std::wcout << pKey->DriverKeyName << std::endl;
                if (targetKey == pKey->DriverKeyName) {
                    return true;
                }
                else return false;
            }

            return false;
        }

        std::wstring get_path(HANDLE h, ULONG index)
        {
            // get name length
            USB_NODE_CONNECTION_NAME name;
            name.ConnectionIndex = index;
            if (!DeviceIoControl(h, IOCTL_USB_GET_NODE_CONNECTION_NAME, &name, sizeof(name), &name, sizeof(name), nullptr, nullptr))
            {
                return std::wstring(L"");
            }

            // alloc space
            if (name.ActualLength < sizeof(name)) return std::wstring(L"");
            auto alloc = std::malloc(name.ActualLength);
            auto pName = std::shared_ptr<USB_NODE_CONNECTION_NAME>(reinterpret_cast<USB_NODE_CONNECTION_NAME *>(alloc), std::free);

            // get name
            pName->ConnectionIndex = index;
            if (DeviceIoControl(h, IOCTL_USB_GET_NODE_CONNECTION_NAME, pName.get(), name.ActualLength, pName.get(), name.ActualLength, nullptr, nullptr))
            {
                return std::wstring(pName->NodeName);
            }

            return std::wstring(L"");
        }

        std::tuple<std::string,usb_spec> handle_usb_hub(const std::wstring & targetKey, const std::wstring & path)
        {
            auto res = std::make_tuple(std::string(""), usb_spec::usb_undefined);

            if (path == L"") return res;
            std::wstring fullPath = L"\\\\.\\" + path;

            HANDLE h = CreateFile(fullPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
            if (h == INVALID_HANDLE_VALUE) return res;
            auto h_gc = std::shared_ptr<void>(h, CloseHandle);

            USB_NODE_INFORMATION info{};
            if (!DeviceIoControl(h, IOCTL_USB_GET_NODE_INFORMATION, &info, sizeof(info), &info, sizeof(info), nullptr, nullptr))
                return res;

            // for each port on the hub
            for (ULONG i = 1; i <= info.u.HubInformation.HubDescriptor.bNumberOfPorts; ++i)
            {
                // allocate something or other
                char buf[sizeof(USB_NODE_CONNECTION_INFORMATION_EX)] = { 0 };
                PUSB_NODE_CONNECTION_INFORMATION_EX pConInfo = reinterpret_cast<PUSB_NODE_CONNECTION_INFORMATION_EX>(buf);

                // get info about port i
                pConInfo->ConnectionIndex = i;
                if (!DeviceIoControl(h, IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX, pConInfo, sizeof(buf), pConInfo, sizeof(buf), nullptr, nullptr))
                {
                    continue;
                }

                // check if device is connected
                if (pConInfo->ConnectionStatus != DeviceConnected)
                {
                    continue; // almost assuredly silently. I think this flag gets set for any port without a device
                }

                // if connected, handle correctly, setting the location info if the device is found
                if (pConInfo->DeviceIsHub)
                    res = handle_usb_hub(targetKey, get_path(h, i));    // Invoke recursion to traverse USB hubs chain
                else
                {
                    if (handle_node(targetKey, h, i)) // exit condition
                    {
                        return std::make_tuple(win_to_utf(fullPath.c_str()) + " " + std::to_string(i),
                                                static_cast<usb_spec>(pConInfo->DeviceDescriptor.bcdUSB));
                    }
                }

                if (std::string("") != std::get<0>(res))  return res;
            }

            return res;
        }

        // Provides Port Id and the USB Specification (USB type)
        bool get_usb_descriptors(uint16_t device_vid, uint16_t device_pid, const std::string& device_uid, std::string& location, usb_spec& spec)
        {
            // Not supported
            return true;
        }


#define MAX_HANDLES 64

        event_base::event_base(HANDLE handle)
            :_handle(handle)
        {}

        event_base::~event_base()
        {
            if (_handle != nullptr)
            {
                CloseHandle(_handle);
                _handle = nullptr;
            }
        }

        bool event_base::set()
        {
            if (_handle == nullptr) return false;
            SetEvent(_handle);
            return true;
        }

        bool event_base::wait(DWORD timeout) const
        {
            if (_handle == nullptr) return false;

            return WaitForSingleObject(_handle, timeout) == WAIT_OBJECT_0; // Return true only if object was signaled
        }

        event_base* event_base::wait(const std::vector<event_base*>& events, bool waitAll, int timeout)
        {
            if (events.size() > MAX_HANDLES) return nullptr; // WaitForMultipleObjects doesn't support waiting on more then 64 handles

            HANDLE handles[MAX_HANDLES];
            auto i = 0;
            for (auto& evnt : events)
            {
                handles[i] = evnt->get_handle();
                ++i;
            }
            auto res = WaitForMultipleObjects(static_cast<DWORD>(events.size()), handles, waitAll, timeout);
            if (res < (WAIT_OBJECT_0 + events.size()))
            {
                return events[res - WAIT_OBJECT_0];
            }
            else
            {
                return nullptr;
            }
        }

        event_base* event_base::wait_all(const std::vector<event_base*>& events, int timeout)
        {
            return wait(events, true, timeout);
        }

        event_base* event_base::wait_any(const std::vector<event_base*>& events, int timeout)
        {
            return wait(events, false, timeout);
        }

        bool manual_reset_event::reset() const
        {
            if (_handle == nullptr) return false;
            return ResetEvent(_handle) != 0;
        }

        manual_reset_event::manual_reset_event()
            :event_base(CreateEvent(nullptr, FALSE, FALSE, nullptr))
        {}

        auto_reset_event::auto_reset_event()
            : event_base(CreateEvent(nullptr, FALSE, FALSE, nullptr))
        {}

        PSECURITY_DESCRIPTOR make_allow_all_security_descriptor(void)
        {
            WCHAR *pszStringSecurityDescriptor;
            pszStringSecurityDescriptor = L"D:(A;;GA;;;WD)(A;;GA;;;AN)S:(ML;;NW;;;ME)";
            PSECURITY_DESCRIPTOR pSecDesc;
            if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
                pszStringSecurityDescriptor, SDDL_REVISION_1, &pSecDesc, nullptr))
                return nullptr;

            return pSecDesc;
        }

        named_mutex::named_mutex(const char* id, unsigned timeout)
            : _timeout(timeout),
            _winusb_mutex(nullptr)
        {
            update_id(id);
        }

        create_and_open_status named_mutex::create_named_mutex(const char* camID)
        {
            CString lstr;
            CString IDstr(camID);
            // IVCAM_DLL string is left in librealsense to allow safe
            // interoperability with existing tools like DCM
            lstr.Format(L"Global\\IVCAM_DLL_WINUSB_MUTEX%s", IDstr);
            auto pSecDesc = make_allow_all_security_descriptor();
            if (pSecDesc)
            {
                SECURITY_ATTRIBUTES SecAttr;
                SecAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
                SecAttr.lpSecurityDescriptor = pSecDesc;
                SecAttr.bInheritHandle = FALSE;

                _winusb_mutex = CreateMutex(
                    &SecAttr,
                    FALSE,
                    lstr);
                LocalFree(pSecDesc);
            }
            //CreateMutex failed
            if (_winusb_mutex == nullptr)
            {
                return Mutex_TotalFailure;
            }
            else if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                CloseHandle(_winusb_mutex);
                _winusb_mutex = NULL;
                return Mutex_AlreadyExist;
            }
            return Mutex_Succeed;
        }

        create_and_open_status named_mutex::open_named_mutex(const char* camID)
        {
            CString lstr;
            CString IDstr(camID);
            // IVCAM_DLL string is left in librealsense to allow safe
            // interoperability with existing tools like DCM
            lstr.Format(L"Global\\IVCAM_DLL_WINUSB_MUTEX%s", IDstr.GetString());

            _winusb_mutex = OpenMutex(
                MUTEX_ALL_ACCESS,            // request full access
                FALSE,                       // handle not inheritable
                lstr);  // object name

            if (_winusb_mutex == nullptr)
            {
                return Mutex_TotalFailure;
            }
            else if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                return Mutex_AlreadyExist;
            }

            return Mutex_Succeed;
        }

        void named_mutex::update_id(const char* camID)
        {
            auto stsCreateMutex = Mutex_Succeed;
            auto stsOpenMutex = Mutex_Succeed;

            if (_winusb_mutex == nullptr)
            {

                for (int i = 0; i < CREATE_MUTEX_RETRY_NUM; i++)
                {
                    stsCreateMutex = create_named_mutex(camID);

                    switch (stsCreateMutex)
                    {
                    case Mutex_Succeed: return;
                    case Mutex_TotalFailure:
                        throw std::runtime_error("CreateNamedMutex returned Mutex_TotalFailure");
                    case Mutex_AlreadyExist:
                    {
                        stsOpenMutex = open_named_mutex(camID);

                        //if OpenMutex failed retry to create the mutex
                        //it can caused by termination of the process that created the mutex
                        if (stsOpenMutex == Mutex_TotalFailure)
                        {
                            continue;
                        }
                        else if (stsOpenMutex == Mutex_Succeed)
                        {
                            return;
                        }
                        else
                        {
                            throw std::runtime_error("OpenNamedMutex returned error " + stsOpenMutex);
                        }
                    }
                    default:
                        break;
                    };
                }
                throw std::runtime_error("Open mutex failed!");
            }
            //Mutex is already exist this mean that
            //the mutex already opened by this process and the method called again after connect event.
            else
            {
                for (auto i = 0; i < CREATE_MUTEX_RETRY_NUM; i++)
                {
                    auto tempMutex = _winusb_mutex;
                    stsCreateMutex = create_named_mutex(camID);

                    switch (stsCreateMutex)
                    {
                        //if creation succeed this mean that new camera connected
                        //and we need to close the old mutex
                    case Mutex_Succeed:
                    {
                        auto res = CloseHandle(tempMutex);
                        if (!res)
                        {
                            throw std::runtime_error("CloseHandle failed");
                        }
                        return;
                    }
                    case Mutex_TotalFailure:
                    {
                        throw std::runtime_error("CreateNamedMutex returned Mutex_TotalFailure");
                    }
                    //Mutex already created by:
                    // 1. This process - which mean the same camera connected.
                    // 2. Other process created the mutex.
                    case Mutex_AlreadyExist:
                    {
                        stsOpenMutex = open_named_mutex(camID);

                        if (stsOpenMutex == Mutex_TotalFailure)
                        {
                            continue;
                        }
                        else if (stsOpenMutex == Mutex_Succeed)
                        {
                            return;
                        }
                        else
                        {
                            throw std::runtime_error("OpenNamedMutex failed with error " + stsOpenMutex);
                        }
                    }
                    default:
                        break;
                    }
                }

                throw std::runtime_error("Open mutex failed!");
            }
        }

        bool named_mutex::try_lock() const
        {
            return (WaitForSingleObject(_winusb_mutex, _timeout) == WAIT_TIMEOUT) ? false : true;
        }

        void named_mutex::acquire() const
        {
            if (!try_lock())
            {
                throw std::runtime_error("Acquire failed!");
            }
        }

        void named_mutex::release() const
        {
            auto sts = ReleaseMutex(_winusb_mutex);
            if (!sts)
            {
                throw std::runtime_error("Failed to release winUsb named Mutex! LastError: " + GetLastError());
            }
        }

        named_mutex::~named_mutex()
        {
            close();
        }

        void named_mutex::close()
        {
            if (_winusb_mutex != nullptr)
            {
                CloseHandle(_winusb_mutex);
                _winusb_mutex = nullptr;
            }
        }

    }
}

#endif
