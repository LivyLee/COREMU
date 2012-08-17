#!/usr/bin/env ruby

require './coremu'

COREMU.check_args

COREMU.run_arm ARGV[0], :replay

