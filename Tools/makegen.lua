--TODO
--Command line arguments
--Stuff Faustus tells me to do
--Clean and optimize
--Top level function to build and link multiple projects.


--USAGE:
--Script will assume working folder to be the rule. Dependencies are gathered
--from implementation file.

--tables
nameTable = {}
fileTable = {}
targetTable = {}


--Navigation functions

--System functions

--returns name of current folder
function currDir()

  os.execute("cd > cd.tmp")

  local f = io.open("cd.tmp", r)
  local cwd = f:read("*a")

  f:close()
  os.remove("cd.tmp")

  cwd = string.gsub(cwd,"\n","")

  while string.find(cwd,"\\") do
		cwd = string.gsub(cwd,string.sub(cwd,1,string.find(cwd,"\\")),"")
			end

  return cwd

end

--returns list of files
function files()

	os.execute("dir /A:-d /b > dir.tmp")

	local f = io.open("dir.tmp","r")
	local cwd = f:read("*a")

	f:close()
	os.remove("dir.tmp")

	return cwd

end

--Initialization

function fillNameTable()

	local nameTable = {}

	os.execute("dir /A:-d /b > dir.tmp")

	local f = io.open("dir.tmp","r")

	for line in f:lines() do

		line = string.gsub(line,".cpp","")
		line = string.gsub(line,".h","")

		if nameTable[line] == nil then
			nameTable[line]=1
		end

	end

	return nameTable

end


function fillFileTable()

	local fileTable = {}

	os.execute("dir /A:-d /b > dir.tmp")

	local f = io.open("dir.tmp","r")

	for line in f:lines() do

	    if string.find(line,".cpp") then
			fileTable[string.gsub(line,".cpp","")]=line
		end

		--if fileTable[string.gsub(line,".h","")] ~= nil then
		--	fileTable[string.gsub(line,".h","")]=fileTable[string.gsub(line,".h","")].." "..line
		--elseif string.find(line,".h") then
		--	fileTable[string.gsub(line,".h","")]=line
		--end

	end

	return fileTable

end


function fillTargetTable(files)

	local targetTable = {}

	for i,v in pairs(files) do
		targetTable[i]=string.gsub(v,"cpp","o")
	end

	return targetTable

end

--other functions

--gather dependences from #includes
function findDepends(file)

	local f = io.open(file,"r")
	local t

	dependencies = {}


	for line in io.lines(file) do
	    if string.find(line,"#include") then
			t = string.gsub(line,"#include","")
		    table.insert(dependencies,t)
	    end
	end

	return dependencies

end

--filter comments, brackets, and standard libraries
function filter(depends)

	--I have no clue why this is necessary, but it seems to skip over key values every once and a while
   for k,v in pairs(depends) do
		if string.find(v,"<") then
			table.remove(depends,i)
		end
   end

   for k,v in pairs(depends) do
		if string.find(v,"<") then
			table.remove(depends,i)
		end
   end

      for k,v in pairs(depends) do
		if string.find(v,"<") then
			table.remove(depends,i)
		end
   end


    local t=""
	local r=""

   for k,v in pairs(depends) do

		r = v;
		--remove comments
		if string.find(v,"//") then
			r = string.gsub(v,string.sub(v,-string.find(v,"//")),"")
			end
		--remove folder i.e standard/limit
		if string.find(r,"/") then
			r = " "..string.gsub(r,string.sub(r,1,string.find(r,"/")),"")
			end

		t=t..string.gsub(r,"\"","")

		end

	return t

end

--lets go!


--for k,v in pairs(nameTable) do print(k) end
--for k,v in pairs(fileTable) do print(v) end
--for k,v in pairs(targetTable) do print(v) end





function makeMake()

fileTable=fillFileTable()
nameTable=fillNameTable()
targetTable=fillTargetTable(fileTable)

local make = io.open("makeTest","w")

--character counter
counter = string.len(currDir())+3

make:write("#Generated by a crappy makeGen script on "..os.date().."\n#2011 Palestar Inc, Kevin Chen\n\n")


make:write("OBJECTS = ")

for k,v in pairs(targetTable) do

	make:write(v.." ")
	counter = counter + string.len(v)+1

	if counter > 88 then
		make:write("\\\n")
		counter = 0
	end

end

make:write("\n\n"..currDir().." : $(OBJECTS)\n\tg++ -o "..currDir().." $(OBJECTS)\n")

for k,v in pairs(targetTable) do

	make:write(v.." : "..fileTable[k]..filter(findDepends(fileTable[k])).."\n\tg++ -c "..fileTable[k].."\n")

end

make:write("Clean :\n\trm "..currDir().." $(OBJECTS)")
end