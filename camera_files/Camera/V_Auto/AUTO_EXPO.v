module AUTO_EXPO  (
    input           CLK, 
    input			RST_N,
    input           PIXEL_VALID,
    input	[7:0]   PIXEL_IN,
    input           FRAME_NEW,
    output  [23:0]  EXPO_OUT
) ; 

reg [15:0] SUM;
wire [23:0] AVG;

parameter Y_FIXED   = 256; 
parameter F_TARGET  = 128; 
parameter EXPO_MAX  = 24'hFFFFFF; 
parameter IMG_SIZE  = 1024;

parameter STEP_BAD = 20000;
parameter STEP_GOOD = 10000;
parameter STEP_BEST = 1000;

parameter THRESH_BAD = 127;

assign AVG = SUM/IMG_SIZE;

reg [2:0] state;

always @( posedge CLK  ) begin 

    if (!RST_N) begin 
        SUM <= 0;
		  state <= 3'b000;
    end
    else begin
        if(FRAME_NEW == 1'b1) begin
            if(AVG > F_TARGET) begin
                if ((AVG-F_TARGET) >= 64) begin
                    state <= 3'b001;
                end else if (((AVG-F_TARGET) >= 32)) begin
                    state <= 3'b010;
                end if (((AVG-F_TARGET) >= 16)) begin
                    state <= 3'b011;
                end                
            end
            else begin
                if ((F_TARGET-AVG) >= 64) begin
                    state <= 3'b100;
                end else if ((F_TARGET-AVG) >= 32) begin
                    state <= 3'b101;
                end if ((F_TARGET-AVG) >= 16) begin
                    state <= 3'b110;
                end        
            end
            SUM <= 0;
        end 
        else begin
            if(PIXEL_VALID == 1'b1) begin
                SUM <= SUM + {8'h0,PIXEL_IN};
            end
        end

    end 

end

always @( posedge CLK  ) begin 

      case(state)
            3'b000: EXPO_OUT <= 24'h11264;
            3'b001: EXPO_OUT <= EXPO_OUT - STEP_BAD;
            3'b010: EXPO_OUT <= EXPO_OUT - STEP_GOOD;
            3'b011: EXPO_OUT <= EXPO_OUT - STEP_BEST;
            3'b100: EXPO_OUT <= EXPO_OUT + STEP_BAD;
            3'b101: EXPO_OUT <= EXPO_OUT + STEP_GOOD;
            3'b110: EXPO_OUT <= EXPO_OUT + STEP_BEST;
        endcase
           

end

endmodule