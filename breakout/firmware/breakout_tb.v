`timescale 1 ns / 1 ps

module testbench;

    reg             XTAL;
    reg             D_IN0;   
    reg             D_IN1;
    reg             D_IN2;
    reg             D_IN3;
    reg             D_IN4;
    reg             D_IN5;
    reg             D_IN6;
    reg             D_IN7;
    reg     [1:0]   LVDS_IN;

    wire    [7:0]   D_OUT;
    wire    [2:0]   LVDS_OUT;

    wire            I2C_SCL;
    wire            I2C_SDA;

    wire            HARP_CLK_OUT;
    wire            LED;
    wire            USBPU;

    wire            NEOPIX;

    reg             PLL_SIM;
    wire    [3:0]   link_led;
    wire    [3:0]   link_status;

    breakout uut (.*);

    // 16 MHz Clk
    always
    #31.25 XTAL = ~XTAL;

    // LVDS_IN[0]
    always
    #50 LVDS_IN[0] = ~LVDS_IN[0];

    // Get around black box PLL
    always 
    #8.33333333333333333 PLL_SIM = ~PLL_SIM;

    // Hidden host clock
    reg host_clk;
    reg [11:0] host_data;
    reg [11:0] latch_data;
    always
    #4.166666666666666666 host_clk = ~host_clk;

    initial begin
        $dumpfile("breakout_tb.vcd");
        $dumpvars();

        // Clocks
        XTAL = 1;
        LVDS_IN = 1;
        PLL_SIM = 1;
        host_clk = 1;
        host_data = 0;
        latch_data = 12'b000100000000;

        #1000
        host_data = 12'b000100000000;

        #1000
        host_data = 12'b011111110000;

        #1000
        host_data = 12'b101000001111;

        #1000
        host_data = 12'b111111111111;

        #1000 

        #1000000 
        $finish;
    end

    //always @ (posedge LVDS_IN[0]) begin
    //    latch_data <= host_data;
    //end

    // LVDS_IN cycles through addresses and data
    always @ (posedge host_clk) begin
       
        LVDS_IN[1] <= latch_data[11];

        //if (cnt == 48) begin
            latch_data <= {latch_data[10:0], latch_data[11]};
            //cnt <= cnt + 1;
        //end else begin
        //    latch_data <= host_data;
        //    cnt <= 0;
        //end

    end

endmodule
