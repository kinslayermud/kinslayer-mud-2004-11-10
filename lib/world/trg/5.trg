#500
Meat~
0 d 100
food~
mload o 930
give meat %actor.name%
mecho Ok.
~
#501
Ram Gate~
1 c 2
ram~
if %ram% != 15  
if %actor.room% != 19301
osend %actor.name% There is no gate here.
else  
eval ram %ram% + 1
global ram
osend %actor.name% Ok.
osend %actor.name% You start barreling down upon the gate with a large battering ram!
oechoaround %actor.name% %actor.name% starts barreling down upon the gate with a large battering ram!
wait 25
if %ram% != 15  
osend %actor.name% You smash the battering ram into the gate!
oechoaround %actor.name% %actor.name% smashes the battering ram into the gate!
oforce 1726 mecho The gate buckles inwards as it is hit by a battering ram!
else  
osend %actor.name% You smash the battering ram into the gate, leaving the gate in splinters.
oechoaround %actor.name% %actor.name% smashes the battering ram into the gate, leaving the gate in splinters.
oforce 1726 mecho The gate shatters into splinters with one last ram.
wait 1
oload obj 553
oforce %actor.name% get destroyed
oforce %actor.name% drop destroyed
opurge destroyed
wait 1
opurge ram
end
end
else  
osend %actor.name% The gate is already destroyed!
end
~
#502
Break Gate~
2 h 100
~
if %object.vnum% == 553
wdoor 1700 n purge
wdoor 19301 s purge
wdoor 19301 s room 1700
wdoor 1700 n room 19301
else
halt
end
else
halt
end
~
#503
Block CoF Exits~
2 c 100
south~
if %actor.level% < 20
wsend %actor.name% You should not venture in that direction untill you are twenty times your starting level.
else
wechoaround %actor.name% %actor.name% leaves south.
wteleport %actor.name% 1700
wforce %actor.name% look
wechoaround %actor.name% %actor.name% has arrived from the north.
end
~
#505
Torch load~
0 d 100
light~
nod
mload obj 900
give torch %actor.name%
~
#506
Darkfriend Message Send~
0 d 100
pass~
if %second_arg% == message
mload obj 950
mload obj 951
write paper pen 0 A message from %actor.name%:
wait 40
write paper pen 2 %speech%
wait 10
say I will pass this along.
wait 10
em smiles slightly.
mat 506 put message chest
mpurge pen
else
end
~
#507
Darkfriend Receive Message~
0 d 100
are~
if (%second_arg% == there) && (%third_arg% == any) && (%fourth_arg% == messages)
mat 506 get all chest
wait 10
em nods slowly.
wait 10
say Let me check.
wait 10
give all.message %actor.name%
wait 10
say That is all.
else
end
~
#508
Darkside Message Send~
0 d 100
send~
if %third_arg% == message
say Darkfriend = %fifth_arg%
set darkfriend %fifth_arg%
mload obj 950
mload obj 951
write paper pen 0 A message from %actor.name%:
wait 10
write paper pen 5 %speech%
wait 10
say I will pass it along.
wait 10
em hands the message to a waiting raven.
wait 10
mecho The raven flys out a nearby window with the message.
wait 10
mat 507 give message ravenmessagemob
mpurge pen
wait 10
mforce ravenmessagemob mat %darkfriend% mechoaround %darkfriend% A raven swoops down from the sky and drops a message to %darkfriend%.
mforce ravenmessagemob mat %darkfriend% msend %darkfriend% A raven swoops down from the sky and drops a message to you.
mforce ravenmessagemob mat %darkfriend% give message %darkfriend%
wait 10
mforce ravenmessagemob put all.message chest
else
end
~
#509
Yurian Anti-Channel~
0 c 100
channel~
if (%random.7% > 1)
halt
else
msend %actor.alias% Ok.
wait 1
msend %actor.alias% Yurian Stonebow directs his attack at you, interrupting your channeling!
mdamage %actor.alias% 20
msend %actor.alias% Ouch!
mechoaround %actor.alias% Yurian Stonebow directs his attack at %actor.name%!
kill %actor.alias%
end
~
#510
Load Pen for Message Mobs~
0 n 100
~
mload obj 508
~
#511
Seanchan Officer Kicking Ass~
0 k 100
~
set rand %random.6%
if %rand% == 1
bash
elseif %rand% == 2
msend %actor.alias% %self.name% swings his club hard at your head!
mechoaround %actor.alias% %self.name% swings his club hard at %actor.alias%'s head!
msend %actor.alias% OUCH! That really hurt!
mdamage %actor.alias% 40
elseif %rand% == 3
kick
elseif %rand% == 4
msend %actor.alias% %self.name% blocks your attack and counters!
mechoaround %actor.alias% %self.name% blocks %actor.alias%'s attack and counters!
msend %actor.alias% Ouch!
mdamage %actor.alias% 20
else
end
~
#520
Newbie Help~
2 c 100
newbie~
wait 20
wsend %actor.alias% The Guardian of Life tells you 'Hello, %actor.name%, welcome to Kinslayer MUD!
wait 30
wsend %actor.alias% The Guardian of Life tells you 'I suppose you want some help getting started. It is a big world out there and it easy to get confused.'
wait 50
wsend %actor.alias% The Guardian of Life tells you 'If you want additional help, type "inform me" and I shall give you what you need to get started.'
~
#521
Guardian Lecture~
2 c 100
inform~
if (%arg% != me)
wsend %actor.alias% Type "Inform me" for information.
halt
else
end
if (%actor.level% > 20)
halt
else
end
wait 5
wsend %actor.alias% The Guardian of Life tells you 'Ahh, I'm glad you are taking the time to listen. Here goes.'
wait 60
wsend %actor.alias% The Guardian of Life tells you 'This is Kinslayer MUD. Based on "The Wheel of Time" series written by Robert Jordan'
wait 60
wsend %actor.alias% The Guardian of Life coughs. How did he know that?
wait 30
wsend %actor.alias% The Guardian of Life tells you 'Anyways, make sure your name fits the time period of the books, or else the imms may lock your character up!'
wait 100
wsend %actor.alias% The Guardian of Life shivers at the thought.
wait 30
wsend %actor.alias% The Guardian of Life tells you 'To see the commands your character can use, type "Commands"'
wait 150
wsend %actor.alias% The Guardian of Life tells you 'You can also view the different socials, which are quite different, by typing "socials"'
wait 150
wsend %actor.alias% The Guardian of Life tells you 'It is important that you know that you can alias the commands. For example. Type "e" instead of "east"'
wait 100
wsend %actor.alias% The Guardian of Life tells you 'You can gain levels by killing basic animals such as deer, hens, foxes, rabbits, or anything simple like that, and move up to the harder stuff later.'
wait 150
wsend %actor.alias% The Guardian of Life tells you 'But before you do that, make sure you get a kit by typing "kit" in my room.'
wait 70
wsend %actor.alias% The Guardian of Life tells you 'You will also want to practice some skills at the different trainers around the world.'
wait 150
wsend %actor.alias% The Guardian of Life tells you 'Just type "practice" to view the skills you can get from them, and type "practice skillname" to practice the skill you chose.'
wait 150
wsend %actor.alias% The Guardian of Life tells you 'If you need additional help, type "who" to see who is on, and type "tell playername message" to send the player a message for help.'
wait 150
wsend %actor.alias% The Guardian of Life tells you 'You can also type "say message" to send a message to the players within your room.'
wait 150
wsend %actor.alias% The Guardian of Life tells you 'After you gain yourself a level, you can "yell message" to send players a message in your zone, and "narrate message" or "chat message" to send players on your side a message.'
wait 150
wsend %actor.alias% The Guardian of Life tells you 'That covers most everything. Good luck in the world of Kinslayer!'
wait 150
wsend %actor.alias% The Guardian of Life tells you 'Oh, one last thing. You can visit our website at "http://kinslayer.mine.nu" to view other help topics as well as the forums.'
~
#550
q~
0 d 100
do~
if %second_arg% == zyrak || %second_arg% == tulon
wait 5
say NEVER!
else
wait 5
em starts peeling off %second_arg%'s clothes.
wait 5
mecho %second_arg% smiles in enjoyment.
wait 5
mecho %second_arg% seems to enjoy it.
wait 5
mecho %second_arg% kisses a hairy, male whore.
wait 5
em pounces on %second_arg%.
~
#551
Pimpsta!~
0 d 100
go!~
wait 65
say HURRY THE HELL UP AND SCREWEM!
wait 20
say THAT IS MORE LIKE IT!
~
#560
Waygate from inside~
0 c 100
pull~
if %arg% != leaf
msend %actor.name% You don't see that here.
else
if %actor.skill(search)% <= 5
set chance 2
elseif (%actor.skill(search)% > 5) && (%actor.skill(search)% < 14)
set chance 15
elseif (%actor.skill(search)% >= 14) && (%actor.skill(search)% < 27)
set chance 35
elseif (%actor.skill(search)% >= 27) && (%actor.skill(search)% < 50)
set chance 55
elseif (%actor.skill(search)% >= 50) && (%actor.skill(search)% < 75)
set chance 75
elseif (%actor.skill(search)% >= 75) && (%actor.skill(search)% < 90)
set chance 90
elseif (%actor.skill(search)% >= 90) && (%actor.skill(search)% < 98)
set chance 95
elseif %actor.skill(search)% >= 98
set chance 100
else
end
end
if %random.100% < %chance%
msend %actor.name% Ok.
open vinecarveddoor
wait 40
close vinecarveddoor
else
msend %actor.name% You don't see that here.
end
~
#570
score~
2 c 100
score~
wsend %actor.name% You have 1(2) hit and 2(3) movement points.
wsend %actor.name% You have scored 0 exp, 0 quest points, and have 0 gold coins.
wsend %actor.name% You need 90000000 exp to reach your next level and 30000 quest points required to rank.
wsend %actor.name% You have been playing for 1 days and 12 hours.
wsend %actor.name% You have gathered 0 Weave Points so far.
wsend %actor.name% This ranks you as %actor.name%  (level 1) (Clan Idiot Rank 1)
~
#571
stat~
2 c 100
stat~
wsend %actor.name% You are a 101 year old idiot.
wsend %actor.name% Your mood is currently: N00b
wsend %actor.name% Height 12 cm, Weight 423 pounds, Weight carried 20 lbs. 
wsend %actor.name% Offensive Bonus: 4 Dodge Bonus: 1, Parry Bonus: 1
wsend %actor.name% Your abilities are: Str: 2, Int: 0, Wis: 0, Dex: 15, Con: 1
wsend %actor.name% Your armor absorbs 0% damage on average.
wsend %actor.name% You are affected by:
wsend %actor.name% NEWBIE
wsend %actor.name% IDIOT
wsend %actor.name% STUPIDITY
~
#580
Gaidin Only Door~
2 c 100
open~
if %arg% == doorway && %actor.clan% == 1
return 0
elseif %arg% == doorway && %actor.clan% != 1
wecho A small window slides partially open.
wait 15
wecho A voice calls out from the window, 'Go away!'
wait 15
wecho The window slides shut.
else
return 0
~
#581
Greet Tyrak~
2 g 100
~
wait 5
if %actor.name% == Zyrak
wsend zyrak Look at trigger 580 - Zyrak.
else
end
~
#592
Benchmarking~
2 d 100
test~
wecho Start...
set i 10000
while (%i% > 0)
eval i %i% - 1
done
wecho End...
~
$~
