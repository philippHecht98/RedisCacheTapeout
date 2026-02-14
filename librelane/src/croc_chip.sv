// Copyright 2024 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Authors:
// - Philippe Sauter <phsauter@iis.ee.ethz.ch>

module croc_chip import croc_pkg::*; #() (
  `ifdef USE_POWER_PINS
  inout wire IOVDD,
  inout wire IOVSS,
  inout wire VDD,
  inout wire VSS,
  `endif

  inout  wire clk_i,
  inout  wire rst_ni,
  inout  wire ref_clk_i,

  inout  wire [3:0] jtag_inputs_PAD,
  output wire jtag_tdo_o,

  inout  wire uart_rx_i,
  output wire uart_tx_o,

  inout  wire fetch_en_i,
  output wire status_o,

  inout  wire [31:0] gpio_io,
  output wire [3:0] unused_o
); 
    logic soc_clk_i;
    logic soc_rst_ni;
    logic soc_ref_clk_i;
    logic soc_testmode;

    logic soc_jtag_tck_i;
    logic soc_jtag_trst_ni;
    logic soc_jtag_tms_i;
    logic soc_jtag_tdi_i;
    logic soc_jtag_tdo_o;

    logic [3:0] jtag_inputs;
    assign {soc_jtag_tck_i, soc_jtag_trst_ni, soc_jtag_tms_i, soc_jtag_tdi_i} = jtag_inputs;

    logic soc_fetch_en_i;
    logic soc_status_o;

    localparam int unsigned GpioCount = 32;

    logic [GpioCount-1:0] soc_gpio_i;             
    logic [GpioCount-1:0] soc_gpio_o;            
    logic [GpioCount-1:0] soc_gpio_out_en_o; // Output enable signal; 0 -> input, 1 -> output

    sg13g2_IOPadIn pad_clk_i (
      `ifdef USE_POWER_PINS
      .iovdd  (IOVDD),
      .iovss  (IOVSS),
      .vdd    (VDD),
      .vss    (VSS),
      `endif
      .pad(clk_i),
      .p2c(soc_clk_i)
      );

    sg13g2_IOPadIn pad_rst_ni (
      `ifdef USE_POWER_PINS
      .iovdd  (IOVDD),
      .iovss  (IOVSS),
      .vdd    (VDD),
      .vss    (VSS),
      `endif
      .pad(rst_ni),       
      .p2c(soc_rst_ni));

    sg13g2_IOPadIn pad_ref_clk_i (
      `ifdef USE_POWER_PINS
      .iovdd  (IOVDD),
      .iovss  (IOVSS),
      .vdd    (VDD),
      .vss    (VSS),
      `endif
      .pad(ref_clk_i),    
      .p2c(soc_ref_clk_i));
      
    assign soc_testmode_i = '0;

    // JTAG input pad instances
    generate
    for (genvar i=0; i<4; i++) begin : jtag_inputs_io
        sg13g2_IOPadIn in (
            `ifdef USE_POWER_PINS
            .iovdd  (IOVDD),
            .iovss  (IOVSS),
            .vdd    (VDD),
            .vss    (VSS),
            `endif
            .p2c    (jtag_inputs[i]),
            .pad    (jtag_inputs_PAD[i])
        );
    end
    endgenerate

    sg13g2_IOPadOut16mA pad_jtag_tdo_o (
      `ifdef USE_POWER_PINS
      .iovdd  (IOVDD),
      .iovss  (IOVSS),
      .vdd    (VDD),
      .vss    (VSS),
      `endif
      .pad(jtag_tdo_o),   
      .c2p(soc_jtag_tdo_o));

    // UART pads
    sg13g2_IOPadIn pad_uart_rx_i (
      `ifdef USE_POWER_PINS
      .iovdd  (IOVDD),
      .iovss  (IOVSS),
      .vdd    (VDD),
      .vss    (VSS),
      `endif
      .pad(uart_rx_i),    
      .p2c(soc_uart_rx_i));
    sg13g2_IOPadOut16mA pad_uart_tx_o (
      `ifdef USE_POWER_PINS
      .iovdd  (IOVDD),
      .iovss  (IOVSS),
      .vdd    (VDD),
      .vss    (VSS),
      `endif
      .pad(uart_tx_o),    
      .c2p(soc_uart_tx_o));

    // Control/status pad instances
    sg13g2_IOPadIn pad_fetch_en_i (
      `ifdef USE_POWER_PINS
      .iovdd  (IOVDD),
      .iovss  (IOVSS),
      .vdd    (VDD),
      .vss    (VSS),
      `endif
      .pad(fetch_en_i),   
      .p2c(soc_fetch_en_i));
    sg13g2_IOPadOut16mA pad_status_o (
      `ifdef USE_POWER_PINS
      .iovdd  (IOVDD),
      .iovss  (IOVSS),
      .vdd    (VDD),
      .vss    (VSS),
      `endif
      .pad(status_o),     
      .c2p(soc_status_o));

    // GPIO pad instances
    generate
    for (genvar i=0; i<32; i++) begin : pad_gpio_io
        sg13g2_IOPadInOut30mA io (
            `ifdef USE_POWER_PINS
            .iovdd  (IOVDD),
            .iovss  (IOVSS),
            .vdd    (VDD),
            .vss    (VSS),
            `endif
            .c2p    (soc_gpio_o[i]),
            .c2p_en (soc_gpio_out_en_o[i]),
            .p2c    (soc_gpio_i[i]),
            .pad    (gpio_io[i])
        );
    end
    endgenerate

    // Unused pads
    generate
    for (genvar i=0; i<4; i++) begin : pad_unused_o
        sg13g2_IOPadOut16mA o (
            `ifdef USE_POWER_PINS
            .iovdd  (IOVDD),
            .iovss  (IOVSS),
            .vdd    (VDD),
            .vss    (VSS),
            `endif
            .c2p    (soc_status_o),
            .pad    (unused_o[i])
        );
    end
    endgenerate

    // Power/ground pad instances
    generate
    for (genvar i=0; i<4; i++) begin : iovdd_pads
        (* keep *)
        sg13g2_IOPadIOVdd iovdd_pad  (
            `ifdef USE_POWER_PINS
            .iovdd  (IOVDD),
            .iovss  (IOVSS),
            .vdd    (VDD),
            .vss    (VSS)
            `endif
        );
    end
    for (genvar i=0; i<4; i++) begin : iovss_pads
        (* keep *)
        sg13g2_IOPadIOVss iovss_pad  (
            `ifdef USE_POWER_PINS
            .iovdd  (IOVDD),
            .iovss  (IOVSS),
            .vdd    (VDD),
            .vss    (VSS)
            `endif
        );
    end
    for (genvar i=0; i<4; i++) begin : vdd_pads
        (* keep *)
        sg13g2_IOPadVdd vdd_pad  (
            `ifdef USE_POWER_PINS
            .iovdd  (IOVDD),
            .iovss  (IOVSS),
            .vdd    (VDD),
            .vss    (VSS)
            `endif
        );
    end
    for (genvar i=0; i<4; i++) begin : vss_pads
        (* keep *)
        sg13g2_IOPadVss vss_pad  (
            `ifdef USE_POWER_PINS
            .iovdd  (IOVDD),
            .iovss  (IOVSS),
            .vdd    (VDD),
            .vss    (VSS)
            `endif
        );
    end
    endgenerate

  croc_soc #(
    .GpioCount( GpioCount )
  )
  i_croc_soc (
    .clk_i          ( soc_clk_i      ),
    .rst_ni         ( soc_rst_ni     ),
    .ref_clk_i      ( soc_ref_clk_i  ),
    .testmode_i     ( soc_testmode_i ),
    .fetch_en_i     ( soc_fetch_en_i ),
    .status_o       ( soc_status_o   ),

    .jtag_tck_i     ( soc_jtag_tck_i   ),
    .jtag_tdi_i     ( soc_jtag_tdi_i   ),
    .jtag_tdo_o     ( soc_jtag_tdo_o   ),
    .jtag_tms_i     ( soc_jtag_tms_i   ),
    .jtag_trst_ni   ( soc_jtag_trst_ni ),

    .uart_rx_i      ( soc_uart_rx_i ),
    .uart_tx_o      ( soc_uart_tx_o ),

    .gpio_i         ( soc_gpio_i        ),             
    .gpio_o         ( soc_gpio_o        ),            
    .gpio_out_en_o  ( soc_gpio_out_en_o )
  );

endmodule
