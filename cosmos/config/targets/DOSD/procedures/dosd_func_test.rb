#NOOP and RESET commands are tested in app_func_test.rb

require 'osk_system'
require 'app_func_test'

class DosdFuncTest < Cosmos::Test

   include AppFuncTest

   
   def initialize
      super()
      @app = app_func_test_init("DOSD")
   end

   def setup
      status_bar("#{Cosmos::Test.current_test_suite}:#{Cosmos::Test.current_test}:#{Cosmos::Test.current_test_case} - setup")
      puts "Running #{Cosmos::Test.current_test_suite}:#{Cosmos::Test.current_test}:#{Cosmos::Test.current_test_case}"
      wait(2)
   end

   def teardown
      status_bar("teardown")
      puts "Running 		   #{Cosmos::Test.current_test_suite}:#{Cosmos::Test.current_test}:#{Cosmos::Test.current_test_case}"
      wait(2)
   end


   #DOSD needs to be tested supervised. tftp disconnects after detection & cannot get valid count
   def test_detect_cmd
      @app = Osk::flight.app["DOSD"]
      target_hk_str = "#{app.target} #{app.hk_pkt}"
      cmd_valid_cnt = tlm("#{target_hk_str} #{Osk::TLM_STR_CMD_VLD}")
      cmd_error_cnt = tlm("#{target_hk_str} #{Osk::TLM_STR_CMD_ERR}")
      ip = `hostname -I`
      Osk::flight.send_cmd("DOSD","DETECT with APP_STATE 1");

      system("gnome-terminal --title='Injecting Flood Attack...' -e 'sudo timeout 10s netwox 76 -i #{ip} -p 23 -s raw' >/dev/null 2>&1 &");

      wait(10)
      puts (cmd_valid_cnt.to_s + " valid count before")
      puts (tlm("#{target_hk_str} #{Osk::TLM_STR_CMD_VLD}").to_s + " valid count after")
      expected_result = cmd_valid_cnt+1 == tlm("#{target_hk_str} #{Osk::TLM_STR_CMD_VLD}") and (cmd_error_cnt == tlm("#{target_hk_str} #{Osk::TLM_STR_CMD_ERR}")) 

      raise "Failed detect command verification" unless expected_result
   end 


   def helper_method
   end



end

