// Copyright 2024 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Authors:
// - Philippe Sauter <phsauter@iis.ee.ethz.ch>

`include "obi/typedef.svh"

package user_pkg;

  //////////////////
  // User Manager //
  //////////////////
  
  // None


  /////////////////////////////////////
  // User Subordinate Address maps ////
  /////////////////////////////////////

  localparam int unsigned NumUserDomainSubordinates = 2; // ROM + RedisCache

  localparam bit [31:0] UserRomAddrOffset   = croc_pkg::UserBaseAddr; // 32'h2000_0000;
  localparam bit [31:0] UserRomAddrRange    = 32'h0000_1000;          // every subordinate has at least 4KB

  localparam bit [31:0] UserRedisCacheAddrOffset   = UserRomAddrOffset + UserRomAddrRange; // 32'h2000_1000;
  localparam bit [31:0] UserRedisCacheAddrRange    = 32'h0000_1000;

  localparam int unsigned NumDemuxSbrRules  = NumUserDomainSubordinates;  // ROM + RedisCache
  localparam int unsigned NumDemuxSbr       = NumDemuxSbrRules + 1; // additional OBI error


  // Enum for bus indices
  typedef enum int {
    UserError      = 0,
    UserRom        = 1,
    UserRedisCache = 2
  } user_demux_outputs_e;

  // Address rules given to address decoder
  localparam croc_pkg::addr_map_rule_t [NumDemuxSbrRules-1:0] user_addr_map = '{                          // 0: OBI Error (default)
    '{ idx: UserRom,       start_addr: UserRomAddrOffset,       end_addr: UserRomAddrOffset  + UserRomAddrRange  }, // 1: ROM
    '{ idx: UserRedisCache,  start_addr: UserRedisCacheAddrOffset,  end_addr: UserRedisCacheAddrOffset + UserRedisCacheAddrRange   }  // 2: RedisCache
  };

endpackage
