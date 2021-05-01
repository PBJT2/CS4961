
require 'osk_system'
require 'app_func_test'
require 'cosmos'
#require 'osk_global'
#require 'osk_flight'
require 'timeout'

class DosiFuncTest < Cosmos::Test

   include AppFuncTest
   
   def initialize
      super()
      @app = app_func_test_init("DOSI")
   end

   def setup
      status_bar("#{Cosmos::Test.current_test_suite}:#{Cosmos::Test.current_test}:#{Cosmos::Test.current_test_case} - setup")
      puts "Running #{Cosmos::Test.current_test_suite}:#{Cosmos::Test.current_test}:#{Cosmos::Test.current_test_case}"
      wait(2)
   end

   def teardown
      status_bar("teardown")
      puts "Running #{Cosmos::Test.current_test_suite}:#{Cosmos::Test.current_test}:#{Cosmos::Test.current_test_case}"
      wait(2)
   end

    #NOOP and RESET commands are tested in app_func_test.rb
   def test_inject_cmd
      target_hk_str = "#{app.target} #{app.hk_pkt}"
      cmd_valid_cnt = tlm("#{target_hk_str} #{Osk::TLM_STR_CMD_VLD}")

      Osk::flight.send_cmd("DOSI","INJECT")
      #IO.popen('/home/osk/OpenSatKit-master/cfs/apps/dosi/fsw/src/floodcmd')

      detected = false
      time_now = Time.now
      wait_secs = Time.now+10

      while detected == false and Time.now < wait_secs
         file1 = File.open("/sys/class/net/enp0s3/statistics/rx_bytes")
         byte1 = file1.read.to_f
	 wait(1)
         file2 = File.open("/sys/class/net/enp0s3/statistics/rx_bytes")
         byte2 = file2.read.to_f
         rate = (byte2-byte1)/1000
         puts("rate: " + rate.to_s + " KB/sec")

         if rate >= 500.0
            detected = true
            puts "Flood injection successful"
         end #if
      end #while
      
      file1.close
      file2.close
      wait(8) #wait for cmd_valid_cnt to incremenet
      puts cmd_valid_cnt
      puts tlm("#{target_hk_str} #{Osk::TLM_STR_CMD_VLD}")
      valid_cmd_incremented = cmd_valid_cnt+1 == tlm("#{target_hk_str} #{Osk::TLM_STR_CMD_VLD}")

      if(valid_cmd_incremented==false)
         puts ("***WARNING VALID_CMD_CNT: " + valid_cmd_incremented.to_s)
         puts ("DETECTED: " + detected.to_s)
      end
      expected_result = detected

      raise "Failed detect command verification within " + (wait_secs-time_now).to_s + " secs" unless expected_result

   end #end test_inject_cmd

   def helper_method
   end



end

