-- initialization goes here
--libinit, errormessage = package.loadlib("MarioCVPlugin.dll", "libinit")
--if errormessage then
--	print ('Error loading plugin:', errormessage)
--end
--libint()
--msgbox("Hew, it worked!", "Lua Message Box")
require("MarioCVPlugin")
print(square(1.414213598))
print(cube(5))

while true do
   -- Code executed once per frame
 
   joypad.set(1, {right=1, B=1}) -- when he lands on ground, have to let go of jump

   snes9x.frameadvance()


end

-- Cleanup goes here