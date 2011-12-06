#!/usr/bin/env ruby

require 'coremu'

COREMU.check_args

COREMU.run_corey 'record', ARGV[0]
#COREMU.run_corey_autoexit 'record', 4

