`include "common.h"

module stack( 
  input wire clk,
  input wire [4:0] ra,
  output wire [`WIDTH-1:0] rd,
  input wire we,
  input wire [4:0] wa,
  input wire [`WIDTH-1:0] wd);

  reg [`WIDTH-1:0] store[0:7];

  always @(posedge clk)
    if (we)
      store[wa[2:0]] <= wd;

  assign rd = store[ra[2:0]];
endmodule
