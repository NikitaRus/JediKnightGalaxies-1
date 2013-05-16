--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Random functions
	
	To test out functions, add them in this file
	
	Written by BobaFett
--------------------------------------------------]]

local function givenoclip(ply, argc, argv)
	
	if argc < 2 then
		ply.NoClip = !ply.NoClip
	else
		local ply2 = players.GetByID(argv[1])
		ply2.NoClip = !ply2.NoClip
	end
end

cmds.Add("donoclip", givenoclip)


local debugmask = ""

local function debugprint(event)
    local info = debug.getinfo(2, "Sl")
	if debugmask == "" or !string.find(info.short_src, debugmask) then
		return
	end
	print(string.format("Debug: %s - (%s:%i)", event, info.short_src, info.currentline))
end

local function enableDebug(argc, argv)
	debugmask = argv[1] or ""
	debug.sethook(debugprint, "crl")
end

cmds.AddRcon("enabledebug", enableDebug)

local function disableDebug(argc, argv)
	debug.sethook()
end

cmds.AddRcon("disableDebug", disableDebug)

local function threadfunc()
	local i
	for i=1,10 do
		print("Message " .. i)
		thread.Wait(500)
	end
	print("Waiting for signal...")
	thread.WaitForSignal("test")
	print("Received signal!")
	print("Terminating...")
end

local function threadtest(argc, argv)
	thread.Create("test", threadfunc)
end

cmds.AddRcon("threadtest", threadtest)

local function signaltest(argc, argv)
	thread.Signal("test")
end

cmds.AddRcon("signaltest", signaltest)

local function teleport(ply, argc, argv)
	if not ply.IsAdmin then
		ply:SendChat("^1Admin ^5- ^7You are not allowed to use this command")
		return
	end
	if argc < 2 then
		local trace = ply:GetEyeTrace()
		ply:Teleport(trace.EndPos + trace.HitNormal * 25)
	else
		local ply2
		if (type(argv[1]) == "number") then
			ply2 = players.GetByID(tonumber(argv[1]))
		else 
			ply2 = players.GetByName(argv[1])
		end
		if not ply2 then
			ply:SendPrint("Target player not found")
			return
		end
		local trace = ply:GetEyeTrace()
		ply2:Teleport(trace.EndPos + trace.HitNormal * 25)
	end
end

cmds.Add("teleport", teleport)

local function usetarg(ply, argc, argv)
	if not ply.IsAdmin then
		ply:SendChat("^1Admin ^5- ^7You are not allowed to use this command")
		return
	end
	if argc < 2 then
		ply:SendPrint("Please specify a target")
	else
		local target = argv[1]
		local plyent = ply:GetEntity()
		local entlist = ents.GetByName(target)
		local k,v
		for k,v in pairs(entlist) do
			v:Use(plyent, plyent)
		end
	end
end

cmds.Add("usetarg", usetarg)

local function ConvoPanic(ply, argc, argv)
	ply:StopConversation()
end

cmds.Add("convopanic", ConvoPanic)

local function ChatTestCmd(ply, argc, argv)
	ply:SendChat("^7System: ^2Testing...")
end

chatcmds.Add("test", ChatTestCmd)

local function pzkFinish(winner)
	print("Pazaak winner: ", winner)
end

local function PzkTestCmd(ply, argc, argv)
	local pzk = sys.CreatePazaakGame()
	local cards = JKG.Pazaak.Cards
	local opp = ply:GetEyeTrace().Entity
	local cardsel = true
	if opp and not opp:IsPlayer() then
		opp = nil
	else
		opp = opp:ToPlayer()
	end
	
	pzk:SetPlayers(ply, opp)
	if not opp then
		pzk:SetAILevel(1)
		pzk:SetAIName("AI")
	end
	pzk:SetFinishCallback(pzkFinish)
	
	pzk:SetCards(1, {10,10,10,10,10,10,
					 10,10,10,10,10,10,
					 10,10,10,10,10,10,
					 10,10,10,10,10 } )

	pzk:SetCards(2, {10,10,10,10,10,10,
					 10,10,10,10,10,10,
					 10,10,10,10,10,10,
					 10,10,10,10,10 } )
					 
	--[[pzk:SetSideDeck(1, {cards.PZCARD_FLIP_1,
						cards.PZCARD_FLIP_2,
						cards.PZCARD_FLIP_3,
						cards.PZCARD_FLIP_4,
						cards.PZCARD_FLIP_5,
						cards.PZCARD_FLIP_6,
						cards.PZCARD_FLIP_2,
						cards.PZCARD_FLIP_3,
						cards.PZCARD_FLIP_4,
						cards.PZCARD_FLIP_5	
						} )
	]]
	if not opp then
		pzk:SetSideDeck(2, {cards.PZCARD_FLIP_1,
						cards.PZCARD_FLIP_2,
						cards.PZCARD_FLIP_3,
						cards.PZCARD_FLIP_4,
						cards.PZCARD_FLIP_5,
						cards.PZCARD_FLIP_6,
						cards.PZCARD_FLIP_2,
						cards.PZCARD_FLIP_3,
						cards.PZCARD_FLIP_4,
						cards.PZCARD_FLIP_5	
						} )
	end
	pzk:ShowCardSelection(cardsel)
	local success, reason = pzk:StartGame()
	if not success then
		ply:SendPrint("Could not start pazaak: ", reason)
	else
		ply:SendPrint("Pazaak started successfully")
	end
	
end

cmds.Add("pzktest", PzkTestCmd)


local function SliceTestCallback(ply, result, udata)
	print("Slice game results: " .. result .. " (" .. tostring(ply) .. ")")
end

local function SliceTestCmd(ply, argc, argv)
	local slc = sys.CreateSlicingGame()
	
	local ok, reason
	
	if argc < 2 then
		ply:SendPrint("Please select a difficulty level: /slctest ^2easy^7, /slctest ^3medium^7, /slctest ^1hard")
		ply:SendPrint("To create a custom game: /slctest ^5custom^7 <columns> Number of columns, between 1 - 8")
		ply:SendPrint("                                         <rows> Number of rows, between 1 - 8")
		ply:SendPrint("                                         <seurity levels> Number of security levels between 1 - 5")
		ply:SendPrint("                                         <intrusion detection time> (seconds) 0 for no intrusion detection")
		ply:SendPrint("                                         <alarm nodes> Number of alarm nodes")
		ply:SendPrint("                                         <reset nodes> Number of reset nodes")
		ply:SendPrint("                                         <level 1 access nodes>")
		ply:SendPrint("                                         <level 2 access nodes>")
		ply:SendPrint("                                         <level 3 access nodes>")
		ply:SendPrint("                                         <level 4 access nodes>")
		ply:SendPrint("                                         <level 5 access nodes>")
		ply:SendPrint("                                         <warning threshold>")
		return
	else
		if argv[1] == 'easy' then
			slc:SetFieldSize(5,5)
			slc:SetSecurityLevelCount(2)
			--slc:SetIntrusionDetection(true, 90)
			--alarm nodes, reset nodes, lvl1, lvl2, lvl3, lvl4, lvl5
			slc:SetNodeCounts(8, 1, 2, 3, 0, 0, 0)
			slc:SetWarningThreshold(12)
			slc:AddProgram("PROBE")
			slc:AddProgram("SCANNODE")
			slc:AddProgram("SCANLINE")
		elseif argv[1] == 'medium' then
			slc:SetFieldSize(6,6)
			slc:SetSecurityLevelCount(3)
			slc:SetIntrusionDetection(true, 600)
			--alarm nodes, reset nodes, lvl1, lvl2, lvl3, lvl4, lvl5
			slc:SetNodeCounts(12, 2, 2, 3, 3, 0, 0)
			slc:SetWarningThreshold(12)
			slc:AddProgram("PROBE")
			slc:AddProgram("SCANNODE")
			slc:AddProgram("SCANLINE")
		elseif argv[1] == 'hard' then
			slc:SetFieldSize(8,8)
			slc:SetSecurityLevelCount(5)
			slc:SetIntrusionDetection(true, 600)
			--alarm nodes, reset nodes, lvl1, lvl2, lvl3, lvl4, lvl5
			slc:SetNodeCounts(30, 5, 2, 2, 2, 2, 2)
			slc:SetWarningThreshold(12)
			slc:AddProgram("PROBE")
			slc:AddProgram("SCANNODE")
			slc:AddProgram("SCANLINE")
		elseif argv[1] == 'custom' then
			if argc < 14 then
				ply:SendPrint("Not enough arguments to create a custom game")
				return
			end
			if argc > 14 then
				ply:SendPrint("Too many arguments to create a custom game")
				return
			end
			slc:SetFieldSize(argv[2],argv[3])
			slc:SetSecurityLevelCount(argv[4])
			if argv[5] != 0 then
				slc:SetIntrusionDetection(true, argv[5])
			end
			--alarm nodes, reset nodes, lvl1, lvl2, lvl3, lvl4, lvl5
			slc:SetNodeCounts(argv[6], argv[7], argv[8], argv[9], argv[10], argv[11], argv[12])
			slc:SetWarningThreshold(argv[13])
			slc:AddProgram("PROBE")
			slc:AddProgram("SCANNODE")
			slc:AddProgram("SCANLINE")
		else
			ply:SendPrint("Invalid difficulty level")
			return
		end
	end
	
	--[[slc:SetFieldSize(6,6)
	slc:SetSecurityLevelCount(3)
	--slc:SetIntrusionDetection(true, 90)
	slc:SetNodeCounts(10, 1, 3, 4, 5, 0, 0)
	slc:SetWarningThreshold(8)
	slc:AddProgram("PROBE")
	slc:AddProgram("SCANNODE")
	slc:AddProgram("SCANLINE")]]
	
	slc:SetCallback(SliceTestCallback, nil)
	
	slc:StartGame(ply)
end

cmds.Add("slctest", SliceTestCmd)

local function AddMethod(method, sb, sb2)
	sb:Append(method .. " ")
	if sb:Length() > 120 then
		sb2:Append("\"")
		sb2:Append(sb:ToString())
		sb2:Append("\"\n")
		sb:Clear()
	end
	
end

local function DumpMetatable(meta, sb, sb2, cache)
	local meta = findmetatable(meta)
	local k,v
	if meta then
		for k,v in pairs(meta) do
			if not cache[k] then
				cache[k] = true
				AddMethod(k, sb, sb2)
			end
		end
	end
end

local function MethodDump()
	-- Write all object methods to a file
	local cache = {}
	local sb = sys.CreateStringBuilder()		-- Line stringbuilder
	local sb2 = sys.CreateStringBuilder()		-- File stringbuilder
	
	-- Process all metatables
	DumpMetatable("Vector", sb, sb2, cache)
	DumpMetatable("Cvar", sb, sb2, cache)
	DumpMetatable("Entity", sb, sb2, cache)
	DumpMetatable("EntityFactory", sb, sb2, cache)
	DumpMetatable("NPC", sb, sb2, cache)
	DumpMetatable("Player", sb, sb2, cache)
	
	-- We're all done
	if sb:Length() > 0 then
		sb2:Append("\"")
		sb2:Append(sb:ToString())
		sb2:Append("\";")
	else
		sb2:Append(";")
	end
	file.Write("methoddump.txt", sb2:ToString())
	print("Methods dumped successfully")
end

cmds.AddRcon("methoddump", MethodDump)
