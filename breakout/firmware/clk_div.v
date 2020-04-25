module clk_div # (
    parameter N = 2,
    parameter PULSE = 0
) (
    input   wire        i_clk,
    input   wire        i_reset,
    output  wire        o_clk
);

reg [$clog2(N):0] clk_cnt = 0;

always @ (posedge i_clk) begin

    if (i_reset) begin
        clk_cnt <= 0;

    end else begin
        clk_cnt <= clk_cnt + 1;
        if (clk_cnt >= N - 1) clk_cnt <= 0;
    end
end

assign o_clk = PULSE ? (clk_cnt == 0 ? 1'b1 : 1'b0) : ((clk_cnt < N / 2) ? 1'b1 : 1'b0); 

// Formal verification
`ifdef FORMAL

// Turn assumes into asserts if this is being used by a higher level module
`ifdef CLK_DIV
`define	ASSUME	assume
`else
`define	ASSUME	assert
`endif

initial restrict(i_reset);
initial	f_last_clk = 1'b0;

always @ ($global_clock) begin

    // Force i_clk toggle at every simulation step
	restrict(i_clk == !f_last_clk);
	f_last_clk <= i_clk;

    // Inputs only change on positive edge of i_clk
	if (!$rose(i_clk)) begin
		`ASSUME($stable(i_reset));
    end
end

always @ (posedge i_clk) begin

    if (clk_cnt == 0 && PULSE)
        assert(o_clk == 1'b1);
    else if (clk_cnt > 0 && PULSE)
        assert(o_clk == 1'b0);
    else if (clk_cnt < N / 2 && !PULSE) 
        assert(o_clk == 1'b1);
    else 
        assert(o_clk == 1'b0);

    assert(clk_cnt < N);
end

`endif

endmodule
