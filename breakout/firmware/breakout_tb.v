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

    wire            UART;

    breakout uut (.*);

    // 16-MHz crystal
    initial begin
        XTAL = 0;
        forever #31.25 XTAL = ~XTAL;
    end

    // Frame clock (10 MHz)
    initial begin
        LVDS_IN[0] = 0;
        #100
        LVDS_IN[0] = 1;
        forever #50 LVDS_IN[0] = ~LVDS_IN[0];
    end

    // Synchronous generated clock (PLL stand-in; 60 MHz)
    initial begin
        PLL_SIM = 0;
        #100
        #2.08333333333 // Phase offset of PLL generated clock
        PLL_SIM = 1;
        forever #8.33333333333333333 PLL_SIM = ~PLL_SIM;
    end

    // Hidden host clock (120 MHz)
    reg host_clk;
    initial begin
        host_clk = 0;
        #100
        host_clk = 1;
        forever #4.166666666666666666 host_clk = ~host_clk;
    end

    // Host data
    reg [11:0] host_data;
    reg [11:0] latch_data;

    initial begin
        $dumpfile("breakout_tb.vcd");
        $dumpvars();

        D_IN0 = 0;
        D_IN1 = 0;
        D_IN2 = 0;
        D_IN3 = 0;
        D_IN4 = 0;
        D_IN5 = 0;
        D_IN6 = 0;
        D_IN7 = 0;

        host_data = 0;
        latch_data = 12'b000100000000;

        //#1000
        //host_data = 12'b000100000000;

        //#1000
        //host_data = 12'b011111110000;

        //#1000
        //host_data = 12'b101000001111;

        //#1000
        //host_data = 12'b111111111111;

        //#1000

        #10000
        $finish;
    end

    //always @ (posedge LVDS_IN[0]) begin
    //    latch_data <= host_data;
    //end

    // LVDS_IN cycles through addresses and data
    always @ (posedge host_clk) begin

        LVDS_IN[1] <= latch_data[11];
        latch_data <= {latch_data[10:0], latch_data[11]};

        //if (cnt == 48) begin
            //cnt <= cnt + 1;
        //end else begin
        //    latch_data <= host_data;
        //    cnt <= 0;
        //end

    end

endmodule
