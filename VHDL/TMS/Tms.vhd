library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity Blocking is
    port
    (
        car : in std_logic_vector(2 downto 0); --[straight, right, left];
  	posit : in std_logic_vector(1 downto 0); --"00" North, "01" East, "10" South, "11" West;
        No, So, Eo, Wo : out std_logic_vector (2 downto 0) --[straight, right, left]
    );
end Blocking;

architecture northBehaviour of Blocking is
signal d, r, o, l : std_logic_vector(2 downto 0); --direct right opposite left
  
  begin
process is
 begin
  r <= "010";
  d <= "111";
    if car="001" then
        o <= "000";
        l <= "000";
    elsif car="010" then
        o <= "110";
        l <= "111";
    elsif car="100" then
        o <= "110";
        l <= "000";
    end if;
        case posit is
            when "00" =>
                No <= d;
                So <= o;
                Eo <= r;
                Wo <= l;
            when "01" =>
                No <= l;
                So <= r;
                Eo <= d;
                Wo <= o;
            when "10" =>
                No <= o;
                So <= d;
                Eo <= l;
                Wo <= r;
            when "11" =>
                No <= r;
                So <= l;
                Eo <= o;
                Wo <= d;
	    when others =>
		No <= "000";
                So <= "000";
                Eo <= "000";
                Wo <= "000";
        end case;
	wait for 1 ps;
end process;
end;
