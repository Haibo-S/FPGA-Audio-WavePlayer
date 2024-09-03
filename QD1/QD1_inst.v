	QD1 u0 (
		.audio_i2c_SDAT             (<connected-to-audio_i2c_SDAT>),             //     audio_i2c.SDAT
		.audio_i2c_SCLK             (<connected-to-audio_i2c_SCLK>),             //              .SCLK
		.audio_mclk_clk             (<connected-to-audio_mclk_clk>),             //    audio_mclk.clk
		.audio_out_ADCDAT           (<connected-to-audio_out_ADCDAT>),           //     audio_out.ADCDAT
		.audio_out_ADCLRCK          (<connected-to-audio_out_ADCLRCK>),          //              .ADCLRCK
		.audio_out_BCLK             (<connected-to-audio_out_BCLK>),             //              .BCLK
		.audio_out_DACDAT           (<connected-to-audio_out_DACDAT>),           //              .DACDAT
		.audio_out_DACLRCK          (<connected-to-audio_out_DACLRCK>),          //              .DACLRCK
		.button_pio_export          (<connected-to-button_pio_export>),          //    button_pio.export
		.clk_50_clk                 (<connected-to-clk_50_clk>),                 //        clk_50.clk
		.egm_interface_stimulus     (<connected-to-egm_interface_stimulus>),     // egm_interface.stimulus
		.egm_interface_response     (<connected-to-egm_interface_response>),     //              .response
		.egm_interface_egm_leds     (<connected-to-egm_interface_egm_leds>),     //              .egm_leds
		.lcd_display_RS             (<connected-to-lcd_display_RS>),             //   lcd_display.RS
		.lcd_display_RW             (<connected-to-lcd_display_RW>),             //              .RW
		.lcd_display_data           (<connected-to-lcd_display_data>),           //              .data
		.lcd_display_E              (<connected-to-lcd_display_E>),              //              .E
		.led_pio_export             (<connected-to-led_pio_export>),             //       led_pio.export
		.reset_n_reset_n            (<connected-to-reset_n_reset_n>),            //       reset_n.reset_n
		.response_out_export        (<connected-to-response_out_export>),        //  response_out.export
		.sdram_0_addr               (<connected-to-sdram_0_addr>),               //       sdram_0.addr
		.sdram_0_ba                 (<connected-to-sdram_0_ba>),                 //              .ba
		.sdram_0_cas_n              (<connected-to-sdram_0_cas_n>),              //              .cas_n
		.sdram_0_cke                (<connected-to-sdram_0_cke>),                //              .cke
		.sdram_0_cs_n               (<connected-to-sdram_0_cs_n>),               //              .cs_n
		.sdram_0_dq                 (<connected-to-sdram_0_dq>),                 //              .dq
		.sdram_0_dqm                (<connected-to-sdram_0_dqm>),                //              .dqm
		.sdram_0_ras_n              (<connected-to-sdram_0_ras_n>),              //              .ras_n
		.sdram_0_we_n               (<connected-to-sdram_0_we_n>),               //              .we_n
		.sdram_clk_clk              (<connected-to-sdram_clk_clk>),              //     sdram_clk.clk
		.segment_drive_segment_data (<connected-to-segment_drive_segment_data>), // segment_drive.segment_data
		.segment_drive_digit1       (<connected-to-segment_drive_digit1>),       //              .digit1
		.segment_drive_digit2       (<connected-to-segment_drive_digit2>),       //              .digit2
		.stimulus_in_export         (<connected-to-stimulus_in_export>),         //   stimulus_in.export
		.spi_master_cs              (<connected-to-spi_master_cs>),              //    spi_master.cs
		.spi_master_sclk            (<connected-to-spi_master_sclk>),            //              .sclk
		.spi_master_mosi            (<connected-to-spi_master_mosi>),            //              .mosi
		.spi_master_miso            (<connected-to-spi_master_miso>),            //              .miso
		.spi_master_cd              (<connected-to-spi_master_cd>),              //              .cd
		.spi_master_wp              (<connected-to-spi_master_wp>),              //              .wp
		.switch_pio_export          (<connected-to-switch_pio_export>),          //    switch_pio.export
		.uart_rxd                   (<connected-to-uart_rxd>),                   //          uart.rxd
		.uart_txd                   (<connected-to-uart_txd>)                    //              .txd
	);

