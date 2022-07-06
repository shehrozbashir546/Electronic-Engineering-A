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

architecture Behaviour of Blocking is
--set the crossroad in the perspective of the main car
signal d, r, o, l : std_logic_vector(2 downto 0); --direct right opposite left
  
  begin
process(car,posit) is
 begin
--assign the allowed turns to every direction in cars perspective

  r <= "010";			--right side is always allowed to turn right
  d <= "111";			--same direction is always allowed to move any direction

    if car="001" then 		--turning left
        o <= "000";
        l <= "000";
    elsif car="010" then  	--turning right
        o <= "110";
        l <= "111";
    elsif car="100" then  	--going straught
        o <= "110";
        l <= "000";
    end if;
        case posit is
            when "00" =>	--Main car coming from North
                No <= d;
                So <= o;
                Eo <= r;
                Wo <= l;
            when "01" =>	--Main car coming from East
                No <= l;
                So <= r;
                Eo <= d;
                Wo <= o;
            when "10" =>	--Main car coming from South
                No <= o;
                So <= d;
                Eo <= l;
                Wo <= r;
            when "11" =>	--Main car coming from West
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
