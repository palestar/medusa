; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
ClassCount=16
ResourceCount=0
NewFileInclude1=#include "stdafx.h"
Class1=CRecordUsers
LastClass=CRecordServers
LastTemplate=CRecordset
Class2=CRecordUserData
Class3=CRecordIgnores
Class4=CRecordFriends
Class5=CRecordGames
Class6=CRecordServers
Class7=CRecordSessions
Class8=CRecordClans
Class9=CRecordClanMembers
Class10=CRecordClanData
Class11=CRecordBanned
Class12=CRecordUserStatus
Class13=CRecordMessages
Class14=CRecordRooms
Class15=CRecordChat
Class16=CRecordRoomMembers

[CLS:CRecordUsers]
Type=0
HeaderFile=RecordUsers.h
ImplementationFile=RecordUsers.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CRecordUsers

[DB:CRecordUsers]
DB=1
DBType=ODBC
ColumnCount=26
Column1=[user_id], 4, 4
Column2=[username], 12, 40
Column3=[user_regdate], 12, 20
Column4=[user_password], 12, 32
Column5=[user_email], 12, 50
Column6=[user_icq], 12, 15
Column7=[user_website], 12, 100
Column8=[user_occ], 12, 100
Column9=[user_from], 12, 100
Column10=[user_intrest], 12, 150
Column11=[user_sig], 12, 255
Column12=[user_viewemail], -6, 1
Column13=[user_theme], 4, 4
Column14=[user_aim], 12, 18
Column15=[user_yim], 12, 25
Column16=[user_msnm], 12, 25
Column17=[user_posts], 4, 4
Column18=[user_attachsig], 4, 4
Column19=[user_desmile], 4, 4
Column20=[user_html], 4, 4
Column21=[user_bbcode], 4, 4
Column22=[user_rank], 4, 4
Column23=[user_level], 4, 4
Column24=[user_lang], 12, 255
Column25=[user_actkey], 12, 32
Column26=[user_newpasswd], 12, 32

[CLS:CRecordUserData]
Type=0
HeaderFile=RecordUserData.h
ImplementationFile=RecordUserData.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CRecordUserData

[DB:CRecordUserData]
DB=1
DBType=ODBC
ColumnCount=12
Column1=[data_id], 4, 4
Column2=[user_id], 4, 4
Column3=[release], -6, 1
Column4=[beta], -6, 1
Column5=[demo], -6, 1
Column6=[moderator], -6, 1
Column7=[server], -6, 1
Column8=[admin], -6, 1
Column9=[data], -4, 65535
Column10=[score], 7, 4
Column11=[rank], 12, 50
Column12=[game_id], 4, 4

[CLS:CRecordIgnores]
Type=0
HeaderFile=RecordIgnores.h
ImplementationFile=RecordIgnores.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r

[DB:CRecordIgnores]
DB=1
DBType=ODBC
ColumnCount=3
Column1=[key_id], 4, 4
Column2=[user_id], 4, 4
Column3=[ignore_id], 4, 4

[CLS:CRecordFriends]
Type=0
HeaderFile=RecordFriends.h
ImplementationFile=RecordFriends.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r

[DB:CRecordFriends]
DB=1
DBType=ODBC
ColumnCount=3
Column1=[key_id], 4, 4
Column2=[user_id], 4, 4
Column3=[friend_id], 4, 4

[CLS:CRecordGames]
Type=0
HeaderFile=RecordGames.h
ImplementationFile=RecordGames.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CRecordGames

[DB:CRecordGames]
DB=1
DBType=ODBC
ColumnCount=8
Column1=[game_id], 4, 4
Column2=[name], 12, 50
Column3=[home_url], 12, 50
Column4=[download_url], 12, 50
Column5=[registry], 12, 50
Column6=[address], 12, 50
Column7=[port], 4, 4
Column8=[valid], -6, 1

[CLS:CRecordServers]
Type=0
HeaderFile=RecordServers.h
ImplementationFile=RecordServers.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CRecordServers

[DB:CRecordServers]
DB=1
DBType=ODBC
ColumnCount=11
Column1=[server_id], 4, 4
Column2=[type], 4, 4
Column3=[flags], 4, 4
Column4=[name], 12, 50
Column5=[description], -1, 65535
Column6=[address], 12, 50
Column7=[port], 4, 4
Column8=[max_clients], 4, 4
Column9=[clients], 4, 4
Column10=[last_updated], 4, 4
Column11=[game_id], 4, 4

[CLS:CRecordSessions]
Type=0
HeaderFile=RecordSessions.h
ImplementationFile=RecordSessions.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CRecordSessions

[DB:CRecordSessions]
DB=1
DBType=ODBC
ColumnCount=4
Column1=[sess_id], 4, 4
Column2=[user_id], 4, 4
Column3=[start_time], 4, 4
Column4=[remote_ip], 12, 15

[DB:CRecordClan]
DB=1
DBType=ODBC
ColumnCount=6
Column1=[clan_id], 4, 4
Column2=[name], 12, 50
Column3=[logo], 12, 50
Column4=[motto], 12, 50
Column5=[home], 12, 50
Column6=[closed], -6, 1

[DB:CRecordClanMembers]
DB=1
DBType=ODBC
ColumnCount=5
Column1=[key_id], 4, 4
Column2=[user_id], 4, 4
Column3=[is_valid], -6, 1
Column4=[is_admin], -6, 1
Column5=[clan_id], 4, 4

[CLS:CRecordClans]
Type=0
HeaderFile=RecordClans.h
ImplementationFile=RecordClans.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CRecordClans

[DB:CRecordClans]
DB=1
DBType=ODBC
ColumnCount=6
Column1=[clan_id], 4, 4
Column2=[name], 12, 50
Column3=[motto], 12, 50
Column4=[home], 12, 50
Column5=[closed], -6, 1
Column6=[approved], -6, 1

[CLS:CRecordClanMembers]
Type=0
HeaderFile=RecordClanMembers.h
ImplementationFile=RecordClanMembers.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CRecordClanMembers

[CLS:CRecordClanData]
Type=0
HeaderFile=RecordClanData.h
ImplementationFile=RecordClanData.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r

[DB:CRecordClanData]
DB=1
DBType=ODBC
ColumnCount=5
Column1=[data_id], 4, 4
Column2=[game_id], 4, 4
Column3=[data], -4, 65535
Column4=[score], 7, 4
Column5=[clan_id], 4, 4

[CLS:CRecordBanned]
Type=0
HeaderFile=RecordBanned.h
ImplementationFile=RecordBanned.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CRecordBanned

[DB:CRecordBanned]
DB=1
DBType=ODBC
ColumnCount=6
Column1=[ban_id], 4, 4
Column2=[ban_userid], 4, 4
Column3=[ban_ip], 12, 16
Column4=[ban_start], 4, 4
Column5=[ban_end], 4, 4
Column6=[ban_time_type], 4, 4

[CLS:CRecordUserStatus]
Type=0
HeaderFile=RecordUserStatus.h
ImplementationFile=RecordUserStatus.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CRecordUserStatus

[DB:CRecordUserStatus]
DB=1
DBType=ODBC
ColumnCount=4
Column1=[status_id], 4, 4
Column2=[status], -1, 65535
Column3=[updated], 4, 4
Column4=[user_id], 4, 4

[CLS:CRecordMessages]
Type=0
HeaderFile=RecordMessages.h
ImplementationFile=RecordMessages.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CRecordMessages

[DB:CRecordMessages]
DB=1
DBType=ODBC
ColumnCount=7
Column1=[msg_id], 4, 4
Column2=[from_userid], 4, 4
Column3=[to_userid], 4, 4
Column4=[msg_time], 12, 20
Column5=[poster_ip], 12, 16
Column6=[msg_status], 4, 4
Column7=[msg_text], -1, 65535

[CLS:CRecordRooms]
Type=0
HeaderFile=RecordRooms.h
ImplementationFile=RecordRooms.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r

[DB:CRecordRooms]
DB=1
DBType=ODBC
ColumnCount=6
Column1=[room_id], 4, 4
Column2=[name], 12, 50
Column3=[is_static], -6, 1
Column4=[is_moderated], -6, 1
Column5=[is_private], -6, 1
Column6=[game_id], 4, 4

[CLS:CRecordChat]
Type=0
HeaderFile=RecordChat.h
ImplementationFile=RecordChat.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CRecordChat

[DB:CRecordChat]
DB=1
DBType=ODBC
ColumnCount=7
Column1=[message_id], 4, 4
Column2=[author], 12, 50
Column3=[author_id], 4, 4
Column4=[time], 4, 4
Column5=[text], -4, 65535
Column6=[recp_id], 4, 4
Column7=[room_id], 4, 4

[CLS:CRecordRoomMembers]
Type=0
HeaderFile=RecordRoomMembers.h
ImplementationFile=RecordRoomMembers.cpp
BaseClass=CRecordset
Filter=N
VirtualFilter=r
LastObject=CRecordRoomMembers

[DB:CRecordRoomMembers]
DB=1
DBType=ODBC
ColumnCount=3
Column1=[member_id], 4, 4
Column2=[user_id], 4, 4
Column3=[room_id], 4, 4

