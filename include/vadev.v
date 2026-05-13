// This File is part of gnucap-qucs
// (C) 2018 Felix Salfelder
// GPLv3+

// mapping qucsator names to actual devices
// "verilog devices" section

//vcresistor:VCR1 _net0 _net1 _net2 _net3 gain="1"
module vcresistor(\1 , \2 , \3 , \4 );
  electrical \1 , \2 , \3 , \4 ;
  inout \1 , \2 , \3 , \4 ;
  parameter real gain=1;

  vcr #(.r(gain)) v(\3 , \4 , \1 , \2 );
endmodule
