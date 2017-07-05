# Halt the flow with an error if the timing constraints weren't met

set minireport [report_timing_summary -no_header -no_detailed_paths -return_string]

if {! [string match -nocase {*timing constraints are met*} $minireport]} {
    send_msg_id showstopper-0 error "Timing constraints weren't met. Please check your design. See Timing Summary Report."
    return -code error
}

if {! [string match -nocase {*There are 0 register/latch pins with no clock*} $minireport]} {
    send_msg_id showstopper-1 error "There are registers with no clock. Please check your design. See Timing Summary Report."
    return -code error
}

if {! [string match -nocase {*There are 0 register/latch pins with constant_clock*} $minireport]} {
    send_msg_id showstopper-2 error "The design is poorly constrained: There are registers assumed to have a constant clock. Please check your design. See Timing Summary Report."
    return -code error
}

if {! [string match -nocase {*There are 0 pins that are not constrained for maximum delay due to constant clock*} $minireport]} {
    send_msg_id showstopper-3 error "The design is poorly constrained: There are internal endpoints that are not constrained due to a constant clock. Please check your design. See Timing Summary Report."
    return -code error
}

if {! [string match -nocase {*There are 0 pins that are not constrained for maximum delay*} $minireport]} {
    send_msg_id showstopper-4 error "The design is poorly constrained: There are internal endpoints that are not constrained. Please check your design. See Timing Summary Report."
    return -code error
}
