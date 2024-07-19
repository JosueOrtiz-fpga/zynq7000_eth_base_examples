# name of platform to be created
set platform_name "z7_echo_server_pform"
# name of processing domain that will be targeted
set domain_name "ps7_cortexa9_0"
# what type of OS will be run on target domain
set os_name "standalone"
# bsp libraries to be added
set domain_bsp_libs "
    lwip220
"
# name of the app to be created
set app_name ""
# name of the template (AMD example app) to import
set template_name "lwIP Echo Server"