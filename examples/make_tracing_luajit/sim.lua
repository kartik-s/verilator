local bit = require("bit")
local top, _, trace = unpack(require("Vtop"))

local t = trace.new()
local dut = top.new()
dut:trace(t, 99)
t:open("logs/vlt_dump.vcd")

dut.reset_l = bit.bnot(0)
dut.clk = 0
dut.in_small = 1
dut.in_quad = 0x1234
dut.in_wide[0] = 0x11111111
dut.in_wide[1] = 0x22222222;
dut.in_wide[2] = 0x3;

local time = 0
t:dump(time)
while not dut:got_finish() do
	time = time + 1
	dut.clk = bit.bnot(dut.clk)
	if dut.clk == 0 then
		if time > 1 and time < 10 then
			dut.reset_l = bit.bnot(1)
		else
			dut.reset_l = bit.bnot(0)
		end
		dut.in_quad = dut.in_quad + 0x12
	end
	dut:eval()
	t:dump(time)
end

t:flush()
t:close()
dut = nil
