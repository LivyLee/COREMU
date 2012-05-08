#!/usr/bin/env ruby

require './coremu'

COREMU.check_args

COREMU.run_linux ARGV[0], :replay

