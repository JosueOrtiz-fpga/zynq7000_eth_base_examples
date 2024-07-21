# Add package: Vitis Python CLI
import vitis

# Get the config file
import sys
sys.path.append(sys.argv[1])
import config

# Create a Vitis client object
client = vitis.create_client()

# Set Vitis Workspace
client.set_workspace(path="./")

# Create the platform
platform = client.create_platform_component(name = config.platform_name,hw_design = sys.argv[2] ,os = config.os_name,cpu = config.cpu_name,domain_name = config.domain_name, no_boot_bsp = False)
# Add the bsp libs
platform = client.get_component(config.platform_name)
domain = platform.get_domain(config.domain_name)
for lib_name, lib_path in config.bsp_libs.items():
    status = domain.set_lib(lib_name=lib_name)
# Build the platform
status = platform.build()

# Create the app component
platform_xpfm=client.find_platform_in_repos(config.platform_name) # This returns the platform xpfm path
# If a template name is provided, ensure it is valid
if config.template_name != None:
    template_list = client.get_templates('EMBD_APP')
    check = config.template_name in template_list
    if check == False:
        raise Exception("Invalid template name provided")

comp = client.create_app_component(name=config.app_name,platform =platform_xpfm,domain = config.domain_name,template = config.template_name)
# Build the app component
comp = client.get_component(name=config.app_name)
comp.build()

# Finish
vitis.dispose()