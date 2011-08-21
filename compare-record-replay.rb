#!/usr/bin/env ruby

require 'pty'
require 'expect'
$expect_verbose = true

require 'coremu'

core = 4

COREMU.setup_corey 'record'
PTY.spawn(COREMU.corey_cmd 'record', core) do |reader, writer, pid|
  reader.expect(/BOOM ! cnt.counter = ([:digit:]*)/) do |r|
    $record_counter = r[1]
    puts "#{$record_counter}"
  end

  reader.expect(/thread_halt: no more threads/) do |r|
    writer.printf("?\C-ax")
  end

  reader.expect(".*")
end

puts '---------------------------------------------------------------'
puts 'record exited'

def wait_fail_or_exit(reader, writer)
  should_break = false
  while true
    reader.expect(/ERROR/, 5) do |r|
      should_break = true if r
    end

    reader.expect(/BOOM ! cnt.counter = ([:digit:]*)/, 5) do |r|
      if r
        puts "got number"
        $replay_counter = r[1]
        puts "#{$replay_counter}"
      end
    end

    reader.expect(/thread_halt: no more threads/, 5) do |r|
      should_break = true if r
    end

    if should_break
      writer.printf("?\C-ax")
      reader.expect(".*")
      break
    end
  end
end

COREMU.setup_corey 'replay'
PTY.spawn(COREMU.corey_cmd 'replay', core) do |reader, writer, pid|
  wait_fail_or_exit(reader, writer)
end

puts '---------------------------------------------------------------'
puts 'replay exited'

if $record_counter != $replay_counter
  puts "!!!!!!Record and replay DIFFERS!!!!!!"
else
  puts "Record and replay ARE THE SAME"
end
