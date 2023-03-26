#include <arpa/inet.h>
#include <cerrno>
#include <ifaddrs.h>
#include <iostream>
#include <net/if.h>
#include <string>
#include <string.h>
#include <sysexits.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <algorithm>

std::string get_dpdk_ip() {
    struct ifaddrs* ptr_ifaddrs = nullptr;

    auto result = getifaddrs(&ptr_ifaddrs);
    if( result != 0 ){
        std::cout << "`getifaddrs()` failed: " << strerror(errno) << std::endl;
        return "NULL";
    }

    for(
        struct ifaddrs* ptr_entry = ptr_ifaddrs;
        ptr_entry != nullptr;
        ptr_entry = ptr_entry->ifa_next
    ){
        std::string ipaddress_human_readable_form;

        std::string interface_name = std::string(ptr_entry->ifa_name);
        sa_family_t address_family = ptr_entry->ifa_addr->sa_family;
        if( address_family == AF_INET ){
            // IPv4

            // Be aware that the `ifa_addr`, `ifa_netmask` and `ifa_data` fields might contain nullptr.
            // Dereferencing nullptr causes "Undefined behavior" problems.
            // So it is need to check these fields before dereferencing.
            if( ptr_entry->ifa_addr != nullptr ){
                char buffer[INET_ADDRSTRLEN] = {0, };
                inet_ntop(
                    address_family,
                    &(reinterpret_cast<struct sockaddr_in*>(ptr_entry->ifa_addr))->sin_addr,
                    buffer,
                    INET_ADDRSTRLEN
                );

                ipaddress_human_readable_form = std::string(buffer);
            }
	        in_addr_t addr;
	        inet_pton(AF_INET, ipaddress_human_readable_form.c_str(), &addr);
	        addr =
                ((addr & 0xff000000) >> 24) |
                ((addr & 0x00ff0000) >>  8) |
                ((addr & 0x0000ff00) <<  8) |
                ((addr & 0x000000ff) << 24);
	        inet_ntop(AF_INET, &addr, &ipaddress_human_readable_form[0], sizeof(ipaddress_human_readable_form));
	        if(ipaddress_human_readable_form.substr(ipaddress_human_readable_form.size() - 3) == ".10") {
                freeifaddrs(ptr_ifaddrs);
                return ipaddress_human_readable_form;
            }
        }
    }
    return "NULL";

}