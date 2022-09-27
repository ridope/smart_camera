
module	VGA_Controller(	
      input	    		   iCLK,
      input	    	   	iRST_N,
      input	    [7:0]	iRed,
      input	    [7:0]	iGreen,
      input	    [7:0]	iBlue,
	  input 	[15:0]	iVideo_W,
	  input 	[15:0]	iVideo_H,
      output      		oRequest,
	  output 			oFrameDone,
      output		[7:0]	oVGA_R,
      output		[7:0]	oVGA_G,
      output		[7:0]	oVGA_B,
      output				oVGA_H_SYNC,
      output				oVGA_V_SYNC,
      output				oVGA_SYNC,
      output				oVGA_BLANK,
      output	reg		[12:0]		H_Cont,
      output	reg		[12:0]		V_Cont						
		);

//=======================================================
// REG/WIRE declarations
//=======================================================
parameter H_MARK   = 16;//MAX 17
parameter H_MARK1  = 10;//MAX 10
parameter V_MARK   = 10; //MAX 9

//	Horizontal Parameter	( Pixel )
parameter	H_SYNC_CYC	=	96;
parameter	H_SYNC_BACK	=	48;
parameter	H_SYNC_ACT	=	640;	
parameter	H_SYNC_FRONT=	16;
parameter	H_SYNC_TOTAL=	800;

//	Virtical Parameter		( Line )
//parameter	V_SYNC_CYC	=	2;
//parameter	V_SYNC_BACK	=	33;

parameter	V_SYNC_CYC	=	2;
parameter	V_SYNC_BACK	=	33 ;
parameter	V_SYNC_ACT	=	480;	
parameter	V_SYNC_FRONT=	10;
parameter	V_SYNC_TOTAL=	525;
//	Start Offset
parameter	X_START		=	H_SYNC_CYC+H_SYNC_BACK;
parameter	Y_START		=	V_SYNC_CYC+V_SYNC_BACK;
parameter	H_BLANK	   =	H_SYNC_FRONT+H_SYNC_CYC+H_SYNC_BACK;
parameter	V_BLANK	   =	V_SYNC_FRONT+V_SYNC_CYC+V_SYNC_BACK;

//=======================================================
// Structural coding
//=======================================================

																  
//---h 								  
always@(posedge iCLK or negedge iRST_N)
begin
	if(!iRST_N)
	begin
		H_Cont		<=	0;
	end
	else
	begin
		if( H_Cont < H_SYNC_TOTAL )
		H_Cont	<=	H_Cont+1;
		else
		H_Cont	<=	0;
	end
end

//	V_Sync Generator, Ref. H_Sync
always@(posedge iCLK or negedge iRST_N)
begin
	if(!iRST_N)
	begin
		V_Cont		<=	0;
	end
	else
	begin
		if(H_Cont==0)
		begin
			if( V_Cont < V_SYNC_TOTAL )
			V_Cont	 <=	V_Cont+1;
			else
			V_Cont	<=	0;
		end
	end
end

//---output 
assign oVGA_BLANK	=   ~((H_Cont < H_BLANK ) || ( V_Cont < V_BLANK ));
assign oVGA_H_SYNC =	( ( H_Cont > (H_SYNC_FRONT-H_MARK1 ) )  &&  ( H_Cont <= (H_SYNC_CYC + H_SYNC_FRONT-H_MARK1)))?0 :1 ; 
assign oVGA_V_SYNC =	( ( V_Cont > (V_SYNC_FRONT ) )  &&  ( V_Cont <= (V_SYNC_CYC + V_SYNC_FRONT)))?0 :1 ; 
//assign oVGA_H_SYNC =	( ( H_Cont > (H_SYNC_BACK ) )  &&  ( H_Cont <= (H_SYNC_CYC + H_SYNC_BACK)))?0 :1 ; 
//assign oVGA_V_SYNC =	( ( V_Cont > (V_SYNC_BACK ) )  &&  ( V_Cont <= (V_SYNC_CYC + V_SYNC_BACK)))?0 :1 ; 

assign oRequest    = (  H_Cont >=  X_START+H_MARK  &&  H_Cont< X_START+iVideo_W+H_MARK 
							  &&
							   V_Cont >=  Y_START+V_MARK && V_Cont< Y_START + iVideo_H + V_MARK)?1:0 ; 

assign oFrameDone = ( (H_Cont ==  X_START+iVideo_W+H_MARK) && (V_Cont ==  Y_START + iVideo_H+V_MARK))? 1:0 ; 
                  
assign	oVGA_SYNC =	 1'b0   ;
assign	oVGA_R	 =	 oVGA_BLANK&oRequest ?	(iRed+iGreen+iBlue)/3	   :	0;
assign	oVGA_G	 =	 oVGA_BLANK&oRequest ?	(iRed+iGreen+iBlue)/3	:	0;
assign	oVGA_B	 =	 oVGA_BLANK&oRequest ?	(iRed+iGreen+iBlue)/3	   :	0;
								  								  
endmodule


