# name of platform to be created
platform_name = "z7_echo_server_pform"
# name of processing domain that will be targeted
domain_name = "ps7_cortexa9_0"
# what type of OS will be run on target domain
os_name = "standalone"
cpu_name = "ps7_cortexa9_0"
domain_name = "standalone_ps7_cortexa9_0"
# bsp libraries: "name" : "path"
bsp_libs = {
    "lwip220": "C:/AMD/Vitis/2024.1/data/embeddedsw/ThirdParty/sw_services/lwip220_v1_0"
}
# name of the app to be created
app_name =  "lwip_echo_server"
# name of the template (AMD example app) to import
template_name =  "lwip_echo_server"