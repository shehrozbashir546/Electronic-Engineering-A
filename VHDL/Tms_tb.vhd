library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity TMS_tb is
end TMS_tb;


architecture Behavior of TMS_tb is

component Blocking
port
    (
        car : in std_logic_vector(2 downto 0); --[straight, right, left];
  	posit : in std_logic_vector(1 downto 0); --"00" North, "01" East, "10" South, "11" West;
        No, So, Eo, Wo : out std_logic_vector (2 downto 0) --[straight, right, left]
    );
end component;
signal CarDir, North, South, East, West : std_logic_vector(2 downto 0); --[straight, right, left];
signal position : std_logic_vector(1 downto 0); --"00" North, "01" East, "10" South, "11" West;

begin
TMSys: Blocking port map(
car => CarDir,
posit => position,
No => North,
So => South,
Wo => West,
Eo => East
);

test: process is

variable test_failed : boolean := false;
begin



CarDir <= "001";
position <= "00";
wait for 10 ps;
if not (North = "111") and not (South = "000") and not (East = "010") and not (West = "000")then
	report "test failed for input combination left North" severity error;
	test_failed := true;
end if;

CarDir <= "010";
position <= "00";
wait for 10 ps;
if not (North = "111") and not (South = "110") and not (East = "010") and not (West = "111")then
	report "test failed for input combination right North" severity error;
	test_failed := true;
end if;

CarDir <= "100";
position <= "00";
wait for 10 ps;
if not (North = "111") and not (South = "110") and not (East = "010") and not (West = "000")then
	report "test failed for input combination straight North" severity error;
	test_failed := true;
end if;

CarDir <= "001";
position <= "01";
wait for 10 ps;
if not (East = "111") and not (West = "000") and not (South = "010") and not (North = "000")then
	report "test failed for input combination left East" severity error;
	test_failed := true;
end if;

CarDir <= "010";
position <= "01";
wait for 10 ps;
if not (East = "111") and not (West = "110") and not (South = "010") and not (North = "111")then
	report "test failed for input combination right East" severity error;
	test_failed := true;
end if;

CarDir <= "100";
position <= "01";
wait for 10 ps;
if not (East = "111") and not (West = "110") and not (South = "010") and not (North = "000")then
	report "test failed for input combination straight East" severity error;
	test_failed := true;
end if;

CarDir <= "001";
position <= "10";
wait for 10 ps;
if not (South = "111") and not (North = "000") and not (West = "010") and not (East = "000")then
	report "test failed for input combination left South" severity error;
	test_failed := true;
end if;

CarDir <= "010";
position <= "10";
wait for 10 ps;
if not (South = "111") and not (North = "110") and not (West = "010") and not (East = "111")then
	report "test failed for input combination right South" severity error;
	test_failed := true;
end if;

CarDir <= "100";
position <= "10";
wait for 10 ps;
if not (South = "111") and not (North = "110") and not (West = "010") and not (East = "000")then
	report "test failed for input combination straight South" severity error;
	test_failed := true;
end if;

CarDir <= "001";
position <= "11";
wait for 10 ps;
if not (West = "111") and not (East = "000") and not (North = "010") and not (South = "000")then
	report "test failed for input combination left West" severity error;
	test_failed := true;
end if;

CarDir <= "010";
position <= "11";
wait for 10 ps;
if not (West = "111") and not (East = "110") and not (North = "010") and not (South = "111")then
	report "test failed for input combination right West" severity error;
	test_failed := true;
end if;

CarDir <= "100";
position <= "11";
wait for 10 ps;
if not (West = "111") and not (East = "110") and not (North = "010") and not (South = "000")then
	report "test failed for input combination straight West" severity error;
	test_failed := true;
end if;

if not test_failed then
		report "test passed!" severity note;
		end if;
end process;

end;