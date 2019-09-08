
`timescale 1 ns / 1 ps

	module cap_filter_v1_0 #
	(
		// Parameters of Axi Master Bus Interface M00_AXIS
		parameter integer C_M00_AXIS_TDATA_WIDTH	= 64,
		parameter integer C_S00_AXIS_TDATA_WIDTH	= 32,
		parameter integer KCAP_ARRAY_SIZE	= 4096,
		parameter integer ADDR_WIDTH = 31
	)
	(
		// Ports of Axi Master Bus Interface M00_AXIS
		input wire  m00_axis_aclk,

		output wire  m00_axis_tvalid,
		output wire [C_M00_AXIS_TDATA_WIDTH-1 : 0] m00_axis_tdata,
//		output wire [(C_M00_AXIS_TDATA_WIDTH/8)-1 : 0] m00_axis_tstrb,
//		output wire  m00_axis_tlast,
		input wire  m00_axis_tready,

		// Ports of Axi Slave Bus Interface S00_AXIS
		input wire  s00_axis_aclk,
		output wire  s00_axis_tready,
		input wire [C_S00_AXIS_TDATA_WIDTH-1 : 0] s00_axis_tdata,
//		input wire [(C_S00_AXIS_TDATA_WIDTH/8)-1 : 0] s00_axis_tstrb,
//		input wire  s00_axis_tlast,
		input wire  s00_axis_tvalid,
		
		output wire [31 : 0] total_kcap_count,
		output wire [31 : 0] valid_kcap_count,
		
	   input wire  axis_aresetn
	);

//    reg [(C_M00_AXIS_TDATA_WIDTH/8)-1 : 0] tstrb = 0;
//    reg m00_tlast = 0;



    reg [31:0] uint32_counter = 0; // counter for read in uint32 values


    wire [15:0] m00_kcap;
    wire [15:0] m00_badge;
    reg [31:0] kcap_array[KCAP_ARRAY_SIZE:0];
//    reg [15:0] read_kcap_index;
//    reg [31:0] write_kcap_index;
    reg [31:0] m00_axis_tdata_reg;
    reg write;
    reg [15:0] read_kcap_total;
    reg [15:0] s00_badge;
  
    
    reg [ADDR_WIDTH:0] read_kcap_index;
    reg [ADDR_WIDTH:0] write_kcap_index;
    
    assign total_kcap_count = read_kcap_total;
    assign valid_kcap_count = read_kcap_index;
      
    
    assign s00_axis_tready = (read_kcap_index < KCAP_ARRAY_SIZE);
    assign m00_axis_tvalid = write;
    
    //assign m00_axis_tdata = { 16'haaaa, m00_badge, m00_kcap << 12};
    assign m00_axis_tdata = { 16'haaaa, m00_badge, 4'h0, m00_kcap, 12'h000};
    assign {m00_badge, m00_kcap} = m00_axis_tdata_reg; 
   
    localparam [15:0] UNUSED = 16'h0000;
    localparam [15:0] INVALID_ID = 16'hffff;

    // output wires
  //  assign m00_axis_tstrb = tstrb;
  //  assign m00_axis_tlast = m00_tlast;

    initial begin
        read_kcap_index = 0;
        read_kcap_total = 0;
        write_kcap_index = 0;
        m00_axis_tdata_reg = 0;
        write = 0;
        s00_badge = 0;
        uint32_counter = 0;
    end

    // read in all capabilities
    always @(posedge s00_axis_aclk) begin
        if (!axis_aresetn) begin
            uint32_counter = 0;
            read_kcap_index = 0;
            read_kcap_total = 0;
        end else if(s00_axis_tvalid && s00_axis_tready) begin
            // every read struct has size of four uint32.
            if(uint32_counter == 4) begin
                uint32_counter = 1;
            end else begin
                uint32_counter = uint32_counter + 1;
            end
            if(uint32_counter == 2) begin
                s00_badge = s00_axis_tdata[31:16];
                if (s00_badge != UNUSED && s00_badge != INVALID_ID) begin
                    kcap_array[read_kcap_index[ADDR_WIDTH-1:0]] = {s00_badge, read_kcap_total};
                    read_kcap_index = read_kcap_index + 1;
                end
                read_kcap_total = read_kcap_total + 1;
            end
        end
    end
    
    
    // write logic 
    always @(posedge m00_axis_aclk) begin
        write = 0;
        if (!axis_aresetn) begin
            write_kcap_index = 0;
        end else if((write_kcap_index < read_kcap_index) && m00_axis_tready) begin
            m00_axis_tdata_reg = kcap_array[write_kcap_index[ADDR_WIDTH-1:0]];
            write_kcap_index = write_kcap_index + 1;
            write = 1;
        end
    end
        
endmodule
