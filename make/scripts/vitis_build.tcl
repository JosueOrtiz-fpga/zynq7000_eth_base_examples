# Set Vitis workspace
setws ./
# Source the parameters defined for design
source [lindex argv 0]
puts "hello world"
# Create Platform Comopnent

# Create application project
# app create -name hello -hw /tmp/wrk/system.xsa -proc ps7_cortexa9_0 -os standalone -lang C -template {Hello World}
# app build -name hello hw_server