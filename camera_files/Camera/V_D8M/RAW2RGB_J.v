
module RAW2RGB_J(	
//---ccd 
input	  [9:0]	 iDATA,
input			    RST,
input           VGA_CLK,
input   [15:0]  LINE_MAX, 
input           VGA_VS ,	
input           VGA_HS ,	 
output	[7:0] oRed,
output 	[7:0] oGreen,
output	[7:0] oBlue,
output	[7:0] bw,
output         oDVAL,
input          framenew

);

parameter D8M_VAL_LINE_MIN  = 1; 

//----- WIRE /REG 
wire	   [9:0]	mDAT0_0;
wire	   [9:0]	mDAT0_1;
wire 		[9:0]	mCCD_R;
wire 		[9:0]	mCCD_G; 
wire 		[9:0]	mCCD_B;
wire	[10:0]	mX_Cont;
wire	[10:0]	mY_Cont;

//-------- RGB OUT ---- 
assign   oRed	 = mCCD_R[9:2];
assign  oGreen  = mCCD_G[9:2] ;
assign	oBlue	 = mCCD_B[9:2];

assign bw = (mCCD_R[9:2]+mCCD_G[9:2]+mCCD_B[9:2])/3;

//--------

VGA_RD_COUNTER  tr( 
  .VGA_CLK      (VGA_CLK     ),
  .VGA_VS       (VGA_VS      ), 
  .READ_Request (VGA_HS), 
  .X_Cont       (mX_Cont      ),
  .Y_Cont       (mY_Cont      )
 
) ;
//----3 2-PORT-LINE-BUFFER----  
Line_Buffer_J 	u0	(	
						.CCD_PIXCLK( VGA_CLK ),
						.mCCD_FVAL ( VGA_VS) ,
                  .mCCD_LVAL ( VGA_HS) , 	
						.X_Cont    ( mX_Cont) , 
						.mCCD_DATA ( iDATA),
						.VGA_CLK   ( VGA_CLK), 
                  .READ_Request (VGA_HS),
                  .VGA_VS    ( VGA_VS),	
                  .READ_Cont ( mX_Cont ),
                  .V_Cont    ( mY_Cont ),
						.taps0x    ( mDAT0_0),
						.taps1x    ( mDAT0_1)
						);					

// Control values to produce a image without any black border
reg    RD_EN ; 
always @( posedge VGA_CLK  )  RD_EN <= ( (mY_Cont > D8M_VAL_LINE_MIN)  && (mX_Cont < LINE_MAX-3)) ? VGA_VS & VGA_HS:0 ; 	

RAW_RGB_BIN  bin(
      .CLK  ( ~VGA_CLK ), 
      .RESET_N ( ~framenew ) , 
      .D0   ( mDAT0_0),
      .D1   ( mDAT0_1),
      .X    ( mX_Cont [0]),
      .Y    ( mY_Cont [0]),
      .iDVAL( RD_EN),
       
      .R    ( mCCD_R),
      .G    ( mCCD_G), 
      .B    ( mCCD_B),
      .oDVAL(oDVAL),
); 


endmodule
