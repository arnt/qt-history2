tell application "Xcode"
	tell project 1
		set inputs to {}
		repeat with targ in targets
			tell targ
				set targname to name of targ
				repeat with bp in every build phase
					set phasename to name of bp
					set fr to absolute path of every build file of bp
					repeat with f in fr
						set end of inputs to targname & ":" & phasename & ":" & f & "
"
					end repeat
				end repeat
			end tell
		end repeat
	end tell
end tell
inputs
