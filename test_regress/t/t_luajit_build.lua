local top = unpack(require("Vt_luajit_build"))

local dut = top.new()

dut.clk = 0
dut:eval()
dut.clk = 1
dut:eval()
