#!/usr/bin/env ruby

require './coremu'

COREMU.check_args

COREMU.run_corey ARGV[0], :record
#COREMU.run_corey_autoexit 'record', 4

