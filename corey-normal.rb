#!/usr/bin/env ruby

require 'coremu'

COREMU.check_args

COREMU.run_corey 'normal', ARGV[0]
