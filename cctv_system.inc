/*
================================================================================
SA-MP CCTV SURVEILLANCE SYSTEM v2.0
================================================================================
Author: LeoXD SAMP++
Compatible: SA-MP 0.3.7+ | All Gamemodes | Windows & Linux
License: MIT License

FEATURES:
- Professional CCTV cameras with 360° rotation
- Auto-rotation with random direction changes  
- Manual camera control (WASD keys + Mobile buttons)
- Multi-owner system (Admin/Police/Faction/House)
- MySQL database integration
- Professional HUD interface with date/time
- Y key access system
- Permission-based access control

INSTALLATION:
1. #include "cctv_system" in your gamemode
2. Create database table using cctv_database.sql
3. Configure variables below according to your gamemode
4. Call CCTV_Load() in OnGameModeInit
5. Compile and enjoy!

COMMANDS:
/addcctv [ownertype] [ownerid] [name] - Add new CCTV
/delcctv [cctvid] - Delete CCTV
/cctvlist - List all CCTVs
/editposcctv [cctvid] - Edit CCTV position
/togglecctv [cctvid] - Toggle CCTV on/off
/gotocctv [cctvid] - Teleport to CCTV
/cctvcek - Check accessible CCTVs
/stopcctv - Stop viewing CCTV

CONTROLS:
Y - Access nearby CCTV
WASD - Manual camera control
Mobile Buttons - Touch controls
Enter/ESC - Exit CCTV viewing
================================================================================

CONFIGURATION - MODIFY THESE ACCORDING TO YOUR GAMEMODE:
*/

// MySQL connection variable - Replace 'g_SQL' with your MySQL connection variable
#define CCTV_MYSQL_CONNECTION g_SQL

// Admin check - Replace with your admin check function/variable
#define CCTV_ADMIN_CHECK(%0) (AccountData[%0][pAdmin] >= 3)

// Notification functions - Replace with your notification functions
#define CCTV_NOTIFICATION(%0,%1,%2) ShowTDN(%0, NOTIFICATION_SUCCESS, %2)
#define CCTV_ERROR_MSG(%0,%1) ShowTDN(%0, NOTIFICATION_ERROR, %1)
#define CCTV_USAGE_MSG(%0,%1) SUM(%0, %1)
#define CCTV_PERMISSION_ERROR(%0) PermissionError(%0)

// HBE system (if your gamemode has HBE) - Comment out if not needed
#define CCTV_HIDE_HBE(%0) HideHBETD(%0)
#define CCTV_SHOW_HBE(%0) ShowHBETD(%0)

// Faction system - Replace with your faction check functions if needed
#define CCTV_GET_PLAYER_FACTION(%0) (0)  // Fallback - no faction check
#define CCTV_GET_PLAYER_HOUSE(%0) (0)    // Fallback - no house check

/*
================================================================================
SYSTEM CODE - DO NOT MODIFY BELOW
================================================================================
*/

#define MAX_CCTV 500
#define CCTV_VIEW_DISTANCE 50.0
#define CCTV_ROTATION_SPEED 2.0
#define CCTV_AUTO_ROTATION_SPEED 3.0
#define CCTV_AUTO_ROTATION_INTERVAL 3000
#define CCTV_AUTO_PITCH_CHANGE -1.0

enum e_cctvinfo
{
    cctvID,
    Float:cctvPos[3],
    Float:cctvAngle,
    cctvWorld,
    cctvInterior,
    cctvOwnerType, // 0 = Admin, 1 = Police, 2 = Faction, 3 = House
    cctvOwnerID,   // FactionID or HouseID
    cctvName[64],
    cctvActive,

    STREAMER_TAG_OBJECT: cctvObject,
    STREAMER_TAG_3D_TEXT_LABEL: cctvLabel
};

new CCTVData[MAX_CCTV][e_cctvinfo];
new Iterator:CCTVs<MAX_CCTV>;

enum e_cctvviewer
{
    bool:isViewingCCTV,
    viewingCCTVID,
    Float:cctvViewAngle,
    Float:cctvViewPitch,
    PlayerText:cctvHUD[7],
    cctvAutoTimer,
    bool:cctvAutoDirection
};

new CCTVViewer[MAX_PLAYERS][e_cctvviewer];

stock CanPlayerAccessCCTV(playerid, cctvid)
{
    if(CCTV_ADMIN_CHECK(playerid)) return 1;
    
    switch(CCTVData[cctvid][cctvOwnerType])
    {
        case 0: return CCTV_ADMIN_CHECK(playerid);
        case 1: return (CCTV_GET_PLAYER_FACTION(playerid) == 1);
        case 2: return (CCTV_GET_PLAYER_FACTION(playerid) == CCTVData[cctvid][cctvOwnerID]);
        case 3: return (CCTV_GET_PLAYER_HOUSE(playerid) == CCTVData[cctvid][cctvOwnerID]);
    }
    return 0;
}

CCTV_Load()
{
    if(CCTV_MYSQL_CONNECTION == MySQL:0 || mysql_errno(CCTV_MYSQL_CONNECTION) != 0)
    {
        print("[CCTV] Error: MySQL connection not available. Skipping CCTV load.");
        return 0;
    }

    new query[256];
    mysql_format(CCTV_MYSQL_CONNECTION, query, sizeof(query), "SELECT * FROM `cctv_data`");
    mysql_tquery(CCTV_MYSQL_CONNECTION, query, "OnCCTVLoad", "");
    return 1;
}

forward OnCCTVLoad();
public OnCCTVLoad()
{
    new rows = cache_num_rows();
    if(rows)
    {
        new cctvid, count = 0;
        for(new i = 0; i < rows; i++)
        {
            cache_get_value_name_int(i, "ID", cctvid);
            
            CCTVData[cctvid][cctvID] = cctvid;
            cache_get_value_name_float(i, "PosX", CCTVData[cctvid][cctvPos][0]);
            cache_get_value_name_float(i, "PosY", CCTVData[cctvid][cctvPos][1]);
            cache_get_value_name_float(i, "PosZ", CCTVData[cctvid][cctvPos][2]);
            cache_get_value_name_float(i, "Angle", CCTVData[cctvid][cctvAngle]);
            cache_get_value_name_int(i, "World", CCTVData[cctvid][cctvWorld]);
            cache_get_value_name_int(i, "Interior", CCTVData[cctvid][cctvInterior]);
            cache_get_value_name_int(i, "OwnerType", CCTVData[cctvid][cctvOwnerType]);
            cache_get_value_name_int(i, "OwnerID", CCTVData[cctvid][cctvOwnerID]);
            cache_get_value_name(i, "Name", CCTVData[cctvid][cctvName], 64);
            cache_get_value_name_int(i, "Active", CCTVData[cctvid][cctvActive]);
            
            CCTV_Create(cctvid);
            Iter_Add(CCTVs, cctvid);
            count++;
        }
        printf("[CCTV] Loaded %d CCTV cameras.", count);
    }
    return 1;
}

CCTV_Save(cctvid)
{
    if(CCTV_MYSQL_CONNECTION == MySQL:0 || mysql_errno(CCTV_MYSQL_CONNECTION) != 0)
    {
        printf("[CCTV] Error: MySQL connection not available. Cannot save CCTV ID %d.", cctvid);
        return 0;
    }

    new query[512];
    mysql_format(CCTV_MYSQL_CONNECTION, query, sizeof(query),
        "UPDATE `cctv_data` SET `PosX`='%f', `PosY`='%f', `PosZ`='%f', `Angle`='%f', `World`='%d', `Interior`='%d', `OwnerType`='%d', `OwnerID`='%d', `Name`='%e', `Active`='%d' WHERE `ID`='%d'",
        CCTVData[cctvid][cctvPos][0], CCTVData[cctvid][cctvPos][1], CCTVData[cctvid][cctvPos][2],
        CCTVData[cctvid][cctvAngle], CCTVData[cctvid][cctvWorld], CCTVData[cctvid][cctvInterior],
        CCTVData[cctvid][cctvOwnerType], CCTVData[cctvid][cctvOwnerID], CCTVData[cctvid][cctvName],
        CCTVData[cctvid][cctvActive], CCTVData[cctvid][cctvID]
    );
    mysql_tquery(CCTV_MYSQL_CONNECTION, query);
    return 1;
}

CCTV_Create(cctvid)
{
    if(CCTVData[cctvid][cctvActive])
    {
        CCTVData[cctvid][cctvObject] = CreateDynamicObject(1886,
            CCTVData[cctvid][cctvPos][0], CCTVData[cctvid][cctvPos][1], CCTVData[cctvid][cctvPos][2],
            0.0, 0.0, CCTVData[cctvid][cctvAngle],
            CCTVData[cctvid][cctvWorld], CCTVData[cctvid][cctvInterior], -1, 100.0);

        new labeltext[128];
        format(labeltext, sizeof(labeltext), "{00FF00}CCTV: %s\n{FFFFFF}Press {FFFF00}Y {FFFFFF}to access", CCTVData[cctvid][cctvName]);
        CCTVData[cctvid][cctvLabel] = CreateDynamic3DTextLabel(labeltext, 0xFFFFFFFF,
            CCTVData[cctvid][cctvPos][0], CCTVData[cctvid][cctvPos][1], CCTVData[cctvid][cctvPos][2] + 0.5,
            10.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0,
            CCTVData[cctvid][cctvWorld], CCTVData[cctvid][cctvInterior], -1, 100.0);
    }
    return 1;
}

CCTV_Destroy(cctvid)
{
    if(IsValidDynamicObject(CCTVData[cctvid][cctvObject]))
        DestroyDynamicObject(CCTVData[cctvid][cctvObject]);
    
    if(IsValidDynamic3DTextLabel(CCTVData[cctvid][cctvLabel]))
        DestroyDynamic3DTextLabel(CCTVData[cctvid][cctvLabel]);
    
    CCTVData[cctvid][cctvObject] = STREAMER_TAG_OBJECT:INVALID_STREAMER_ID;
    CCTVData[cctvid][cctvLabel] = STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID;
    return 1;
}

GetPlayerNearestCCTV(playerid)
{
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
    foreach(new cctvid : CCTVs)
    {
        if(CCTVData[cctvid][cctvActive] && 
           GetPlayerVirtualWorld(playerid) == CCTVData[cctvid][cctvWorld] &&
           GetPlayerInterior(playerid) == CCTVData[cctvid][cctvInterior])
        {
            if(GetPlayerDistanceFromPoint(playerid, CCTVData[cctvid][cctvPos][0], CCTVData[cctvid][cctvPos][1], CCTVData[cctvid][cctvPos][2]) <= 3.0)
            {
                if(CanPlayerAccessCCTV(playerid, cctvid))
                    return cctvid;
            }
        }
    }
    return -1;
}

CCTV_UpdateCameraView(playerid, cctvid)
{
    if(!CCTVViewer[playerid][isViewingCCTV] || cctvid == -1) return 0;

    new Float:lookX, Float:lookY, Float:lookZ;
    new Float:distance = CCTV_VIEW_DISTANCE;

    lookX = CCTVData[cctvid][cctvPos][0] + (distance * floatcos(CCTVViewer[playerid][cctvViewAngle], degrees));
    lookY = CCTVData[cctvid][cctvPos][1] + (distance * floatsin(CCTVViewer[playerid][cctvViewAngle], degrees));
    lookZ = CCTVData[cctvid][cctvPos][2] + CCTVViewer[playerid][cctvViewPitch];

    SetPlayerCameraLookAt(playerid, lookX, lookY, lookZ);
    return 1;
}

CCTV_StartViewing(playerid, cctvid)
{
    if(CCTVViewer[playerid][isViewingCCTV])
        CCTV_StopViewing(playerid);

    if(!CCTVData[cctvid][cctvActive])
    {
        CCTV_ERROR_MSG(playerid, "CCTV ini sedang tidak aktif!");
        return 0;
    }

    CCTVViewer[playerid][isViewingCCTV] = true;
    CCTVViewer[playerid][viewingCCTVID] = cctvid;
    CCTVViewer[playerid][cctvViewAngle] = CCTVData[cctvid][cctvAngle];
    CCTVViewer[playerid][cctvViewPitch] = 0.0;
    CCTVViewer[playerid][cctvAutoDirection] = bool:random(2);

    CCTVViewer[playerid][cctvAutoTimer] = SetTimerEx("CCTV_AutoRotate", CCTV_AUTO_ROTATION_INTERVAL, true, "i", playerid);

    SetPlayerCameraPos(playerid, CCTVData[cctvid][cctvPos][0], CCTVData[cctvid][cctvPos][1], CCTVData[cctvid][cctvPos][2]);
    CCTV_UpdateCameraView(playerid, cctvid);

    TogglePlayerControllable(playerid, false);

    CCTV_HIDE_HBE(playerid);

    CCTV_ShowHUD(playerid);
    CCTV_NOTIFICATION(playerid, 1, "CCTV aktif! Kamera berputar otomatis setiap 3 detik.");
    return 1;
}

CCTV_StopViewing(playerid)
{
    if(!CCTVViewer[playerid][isViewingCCTV]) return 0;

    CCTV_HideHUD(playerid);
    CancelSelectTextDraw(playerid);
    SetCameraBehindPlayer(playerid);
    TogglePlayerControllable(playerid, true);

    if(CCTVViewer[playerid][cctvAutoTimer] != 0)
    {
        KillTimer(CCTVViewer[playerid][cctvAutoTimer]);
        CCTVViewer[playerid][cctvAutoTimer] = 0;
    }

    CCTVViewer[playerid][isViewingCCTV] = false;
    CCTVViewer[playerid][viewingCCTVID] = -1;
    CCTVViewer[playerid][cctvViewAngle] = 0.0;
    CCTVViewer[playerid][cctvViewPitch] = 0.0;
    CCTVViewer[playerid][cctvAutoDirection] = false;

    CCTV_SHOW_HBE(playerid);

    CCTV_NOTIFICATION(playerid, 1, "CCTV viewing dihentikan.");
    return 1;
}

forward CCTV_AutoRotate(playerid);
public CCTV_AutoRotate(playerid)
{
    if(!CCTVViewer[playerid][isViewingCCTV]) return 0;

    new cctvid = CCTVViewer[playerid][viewingCCTVID];
    if(cctvid == -1) return 0;

    if(CCTVViewer[playerid][cctvAutoDirection])
        CCTVViewer[playerid][cctvViewAngle] += CCTV_AUTO_ROTATION_SPEED;
    else
        CCTVViewer[playerid][cctvViewAngle] -= CCTV_AUTO_ROTATION_SPEED;

    if(CCTVViewer[playerid][cctvViewAngle] >= 360.0)
        CCTVViewer[playerid][cctvViewAngle] -= 360.0;
    else if(CCTVViewer[playerid][cctvViewAngle] < 0.0)
        CCTVViewer[playerid][cctvViewAngle] += 360.0;

    if(random(100) < 10)
        CCTVViewer[playerid][cctvAutoDirection] = !CCTVViewer[playerid][cctvAutoDirection];

    CCTV_UpdateCameraView(playerid, cctvid);
    return 1;
}

CCTV_ShowHUD(playerid)
{
    new string[128];
    format(string, sizeof(string), "~r~[CCTV]~w~ %s", CCTVData[CCTVViewer[playerid][viewingCCTVID]][cctvName]);
    CCTVViewer[playerid][cctvHUD][0] = CreatePlayerTextDraw(playerid, 10.0, 10.0, string);
    PlayerTextDrawLetterSize(playerid, CCTVViewer[playerid][cctvHUD][0], 0.35, 1.4);
    PlayerTextDrawAlignment(playerid, CCTVViewer[playerid][cctvHUD][0], 1);
    PlayerTextDrawColor(playerid, CCTVViewer[playerid][cctvHUD][0], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, CCTVViewer[playerid][cctvHUD][0], 1);
    PlayerTextDrawShow(playerid, CCTVViewer[playerid][cctvHUD][0]);

    new year, month, day, hour, minute, second;
    getdate(year, month, day);
    gettime(hour, minute, second);
    format(string, sizeof(string), "~w~%02d/%02d/%d~n~%02d:%02d:%02d", day, month, year, hour, minute, second);
    CCTVViewer[playerid][cctvHUD][1] = CreatePlayerTextDraw(playerid, 10.0, 400.0, string);
    PlayerTextDrawLetterSize(playerid, CCTVViewer[playerid][cctvHUD][1], 0.25, 1.0);
    PlayerTextDrawAlignment(playerid, CCTVViewer[playerid][cctvHUD][1], 1);
    PlayerTextDrawColor(playerid, CCTVViewer[playerid][cctvHUD][1], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, CCTVViewer[playerid][cctvHUD][1], 1);
    PlayerTextDrawShow(playerid, CCTVViewer[playerid][cctvHUD][1]);

    CCTVViewer[playerid][cctvHUD][2] = CreatePlayerTextDraw(playerid, 500.0, 200.0, "~y~<~w~");
    PlayerTextDrawLetterSize(playerid, CCTVViewer[playerid][cctvHUD][2], 0.8, 3.0);
    PlayerTextDrawAlignment(playerid, CCTVViewer[playerid][cctvHUD][2], 1);
    PlayerTextDrawColor(playerid, CCTVViewer[playerid][cctvHUD][2], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, CCTVViewer[playerid][cctvHUD][2], 1);
    PlayerTextDrawUseBox(playerid, CCTVViewer[playerid][cctvHUD][2], true);
    PlayerTextDrawBoxColor(playerid, CCTVViewer[playerid][cctvHUD][2], 0x00000088);
    PlayerTextDrawTextSize(playerid, CCTVViewer[playerid][cctvHUD][2], 530.0, 230.0);
    PlayerTextDrawSetSelectable(playerid, CCTVViewer[playerid][cctvHUD][2], true);
    PlayerTextDrawShow(playerid, CCTVViewer[playerid][cctvHUD][2]);

    CCTVViewer[playerid][cctvHUD][3] = CreatePlayerTextDraw(playerid, 540.0, 200.0, "~y~>~w~");
    PlayerTextDrawLetterSize(playerid, CCTVViewer[playerid][cctvHUD][3], 0.8, 3.0);
    PlayerTextDrawAlignment(playerid, CCTVViewer[playerid][cctvHUD][3], 1);
    PlayerTextDrawColor(playerid, CCTVViewer[playerid][cctvHUD][3], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, CCTVViewer[playerid][cctvHUD][3], 1);
    PlayerTextDrawUseBox(playerid, CCTVViewer[playerid][cctvHUD][3], true);
    PlayerTextDrawBoxColor(playerid, CCTVViewer[playerid][cctvHUD][3], 0x00000088);
    PlayerTextDrawTextSize(playerid, CCTVViewer[playerid][cctvHUD][3], 570.0, 230.0);
    PlayerTextDrawSetSelectable(playerid, CCTVViewer[playerid][cctvHUD][3], true);
    PlayerTextDrawShow(playerid, CCTVViewer[playerid][cctvHUD][3]);

    CCTVViewer[playerid][cctvHUD][4] = CreatePlayerTextDraw(playerid, 520.0, 170.0, "~y~^~w~");
    PlayerTextDrawLetterSize(playerid, CCTVViewer[playerid][cctvHUD][4], 0.8, 3.0);
    PlayerTextDrawAlignment(playerid, CCTVViewer[playerid][cctvHUD][4], 1);
    PlayerTextDrawColor(playerid, CCTVViewer[playerid][cctvHUD][4], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, CCTVViewer[playerid][cctvHUD][4], 1);
    PlayerTextDrawUseBox(playerid, CCTVViewer[playerid][cctvHUD][4], true);
    PlayerTextDrawBoxColor(playerid, CCTVViewer[playerid][cctvHUD][4], 0x00000088);
    PlayerTextDrawTextSize(playerid, CCTVViewer[playerid][cctvHUD][4], 550.0, 200.0);
    PlayerTextDrawSetSelectable(playerid, CCTVViewer[playerid][cctvHUD][4], true);
    PlayerTextDrawShow(playerid, CCTVViewer[playerid][cctvHUD][4]);

    CCTVViewer[playerid][cctvHUD][5] = CreatePlayerTextDraw(playerid, 520.0, 230.0, "~y~v~w~");
    PlayerTextDrawLetterSize(playerid, CCTVViewer[playerid][cctvHUD][5], 0.8, 3.0);
    PlayerTextDrawAlignment(playerid, CCTVViewer[playerid][cctvHUD][5], 1);
    PlayerTextDrawColor(playerid, CCTVViewer[playerid][cctvHUD][5], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, CCTVViewer[playerid][cctvHUD][5], 1);
    PlayerTextDrawUseBox(playerid, CCTVViewer[playerid][cctvHUD][5], true);
    PlayerTextDrawBoxColor(playerid, CCTVViewer[playerid][cctvHUD][5], 0x00000088);
    PlayerTextDrawTextSize(playerid, CCTVViewer[playerid][cctvHUD][5], 550.0, 260.0);
    PlayerTextDrawSetSelectable(playerid, CCTVViewer[playerid][cctvHUD][5], true);
    PlayerTextDrawShow(playerid, CCTVViewer[playerid][cctvHUD][5]);

    CCTVViewer[playerid][cctvHUD][6] = CreatePlayerTextDraw(playerid, 580.0, 10.0, "~r~EXIT~w~");
    PlayerTextDrawLetterSize(playerid, CCTVViewer[playerid][cctvHUD][6], 0.3, 1.5);
    PlayerTextDrawAlignment(playerid, CCTVViewer[playerid][cctvHUD][6], 1);
    PlayerTextDrawColor(playerid, CCTVViewer[playerid][cctvHUD][6], 0xFFFFFFFF);
    PlayerTextDrawSetOutline(playerid, CCTVViewer[playerid][cctvHUD][6], 1);
    PlayerTextDrawUseBox(playerid, CCTVViewer[playerid][cctvHUD][6], true);
    PlayerTextDrawBoxColor(playerid, CCTVViewer[playerid][cctvHUD][6], 0xFF000088);
    PlayerTextDrawTextSize(playerid, CCTVViewer[playerid][cctvHUD][6], 620.0, 35.0);
    PlayerTextDrawSetSelectable(playerid, CCTVViewer[playerid][cctvHUD][6], true);
    PlayerTextDrawShow(playerid, CCTVViewer[playerid][cctvHUD][6]);

    SelectTextDraw(playerid, 0xFFFFFF88);
    return 1;
}

CCTV_HideHUD(playerid)
{
    for(new i = 0; i < 7; i++)
    {
        if(CCTVViewer[playerid][cctvHUD][i] != PlayerText:INVALID_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, CCTVViewer[playerid][cctvHUD][i]);
            CCTVViewer[playerid][cctvHUD][i] = PlayerText:INVALID_TEXT_DRAW;
        }
    }
    return 1;
}

YCMD:addcctv(playerid, params[], help)
{
    if(!CCTV_ADMIN_CHECK(playerid)) return CCTV_PERMISSION_ERROR(playerid);

    new ownertype, ownerid, name[64];
    if(sscanf(params, "dds[64]", ownertype, ownerid, name))
    {
        CCTV_USAGE_MSG(playerid, "/addcctv [ownertype] [ownerid] [name]");
        CCTV_USAGE_MSG(playerid, "Owner Type: 0=Admin, 1=Police, 2=Faction, 3=House");
        return 1;
    }

    if(ownertype < 0 || ownertype > 3)
        return CCTV_ERROR_MSG(playerid, "Owner type harus 0-3!");

    new cctvid = Iter_Free(CCTVs);
    if(cctvid == INVALID_ITERATOR_SLOT)
        return CCTV_ERROR_MSG(playerid, "Maksimal CCTV sudah tercapai!");

    GetPlayerPos(playerid, CCTVData[cctvid][cctvPos][0], CCTVData[cctvid][cctvPos][1], CCTVData[cctvid][cctvPos][2]);
    GetPlayerFacingAngle(playerid, CCTVData[cctvid][cctvAngle]);

    CCTVData[cctvid][cctvID] = cctvid;
    CCTVData[cctvid][cctvPos][2] += 3.0;
    CCTVData[cctvid][cctvWorld] = GetPlayerVirtualWorld(playerid);
    CCTVData[cctvid][cctvInterior] = GetPlayerInterior(playerid);
    CCTVData[cctvid][cctvOwnerType] = ownertype;
    CCTVData[cctvid][cctvOwnerID] = ownerid;
    format(CCTVData[cctvid][cctvName], 64, "%s", name);
    CCTVData[cctvid][cctvActive] = 1;

    if(CCTV_MYSQL_CONNECTION != MySQL:0 && mysql_errno(CCTV_MYSQL_CONNECTION) == 0)
    {
        new query[512];
        mysql_format(CCTV_MYSQL_CONNECTION, query, sizeof(query),
            "INSERT INTO `cctv_data` (`ID`, `PosX`, `PosY`, `PosZ`, `Angle`, `World`, `Interior`, `OwnerType`, `OwnerID`, `Name`, `Active`) VALUES ('%d', '%f', '%f', '%f', '%f', '%d', '%d', '%d', '%d', '%e', '%d')",
            cctvid, CCTVData[cctvid][cctvPos][0], CCTVData[cctvid][cctvPos][1], CCTVData[cctvid][cctvPos][2],
            CCTVData[cctvid][cctvAngle], CCTVData[cctvid][cctvWorld], CCTVData[cctvid][cctvInterior],
            CCTVData[cctvid][cctvOwnerType], CCTVData[cctvid][cctvOwnerID], CCTVData[cctvid][cctvName],
            CCTVData[cctvid][cctvActive]
        );
        mysql_tquery(CCTV_MYSQL_CONNECTION, query);
    }

    CCTV_Create(cctvid);
    Iter_Add(CCTVs, cctvid);

    new string[128];
    format(string, sizeof(string), "CCTV ID %d '%s' berhasil dibuat!", cctvid, name);
    CCTV_NOTIFICATION(playerid, 1, string);
    return 1;
}

YCMD:delcctv(playerid, params[], help)
{
    if(!CCTV_ADMIN_CHECK(playerid)) return CCTV_PERMISSION_ERROR(playerid);

    new cctvid;
    if(sscanf(params, "d", cctvid))
        return CCTV_USAGE_MSG(playerid, "/delcctv [cctvid]");

    if(!Iter_Contains(CCTVs, cctvid))
        return CCTV_ERROR_MSG(playerid, "CCTV ID tidak valid!");

    CCTV_Destroy(cctvid);
    Iter_Remove(CCTVs, cctvid);

    CCTVData[cctvid][cctvID] = 0;
    CCTVData[cctvid][cctvActive] = 0;

    if(CCTV_MYSQL_CONNECTION != MySQL:0 && mysql_errno(CCTV_MYSQL_CONNECTION) == 0)
    {
        new query[128];
        mysql_format(CCTV_MYSQL_CONNECTION, query, sizeof(query), "DELETE FROM `cctv_data` WHERE `ID`='%d'", cctvid);
        mysql_tquery(CCTV_MYSQL_CONNECTION, query);
    }

    new string[64];
    format(string, sizeof(string), "CCTV ID %d berhasil dihapus!", cctvid);
    CCTV_NOTIFICATION(playerid, 1, string);
    return 1;
}

YCMD:cctvlist(playerid, params[], help)
{
    if(!CCTV_ADMIN_CHECK(playerid)) return CCTV_PERMISSION_ERROR(playerid);

    new string[2048], count = 0;
    strcat(string, "ID\tName\tOwner Type\tOwner ID\tActive\n");

    foreach(new cctvid : CCTVs)
    {
        new ownertype_str[16];
        switch(CCTVData[cctvid][cctvOwnerType])
        {
            case 0: ownertype_str = "Admin";
            case 1: ownertype_str = "Police";
            case 2: ownertype_str = "Faction";
            case 3: ownertype_str = "House";
            default: ownertype_str = "Unknown";
        }

        new temp[128];
        format(temp, sizeof(temp), "%d\t%s\t%s\t%d\t%s\n",
            cctvid, CCTVData[cctvid][cctvName], ownertype_str,
            CCTVData[cctvid][cctvOwnerID], (CCTVData[cctvid][cctvActive] ? "Yes" : "No"));
        strcat(string, temp);
        count++;
    }

    if(count == 0)
        strcat(string, "No CCTV cameras found.");

    ShowPlayerDialog(playerid, 9999, DIALOG_STYLE_TABLIST_HEADERS, "CCTV List", string, "Close", "");
    return 1;
}

YCMD:editposcctv(playerid, params[], help)
{
    if(!CCTV_ADMIN_CHECK(playerid)) return CCTV_PERMISSION_ERROR(playerid);

    new cctvid;
    if(sscanf(params, "d", cctvid))
        return CCTV_USAGE_MSG(playerid, "/editposcctv [cctvid]");

    if(!Iter_Contains(CCTVs, cctvid))
        return CCTV_ERROR_MSG(playerid, "CCTV ID tidak valid!");

    CCTVViewer[playerid][isViewingCCTV] = false;
    CCTVViewer[playerid][viewingCCTVID] = cctvid;

    if(IsValidDynamicObject(CCTVData[cctvid][cctvObject]))
        DestroyDynamicObject(CCTVData[cctvid][cctvObject]);

    CCTVData[cctvid][cctvObject] = CreateDynamicObject(1886,
        CCTVData[cctvid][cctvPos][0], CCTVData[cctvid][cctvPos][1], CCTVData[cctvid][cctvPos][2],
        0.0, 0.0, CCTVData[cctvid][cctvAngle],
        CCTVData[cctvid][cctvWorld], CCTVData[cctvid][cctvInterior], -1, 100.0);

    EditDynamicObject(playerid, CCTVData[cctvid][cctvObject]);
    CCTV_NOTIFICATION(playerid, 1, "Edit mode aktif! Gunakan mouse untuk menggeser CCTV, ESC untuk selesai.");
    return 1;
}

YCMD:togglecctv(playerid, params[], help)
{
    if(!CCTV_ADMIN_CHECK(playerid)) return CCTV_PERMISSION_ERROR(playerid);

    new cctvid;
    if(sscanf(params, "d", cctvid))
        return CCTV_USAGE_MSG(playerid, "/togglecctv [cctvid]");

    if(!Iter_Contains(CCTVs, cctvid))
        return CCTV_ERROR_MSG(playerid, "CCTV ID tidak valid!");

    CCTVData[cctvid][cctvActive] = !CCTVData[cctvid][cctvActive];

    if(CCTVData[cctvid][cctvActive])
    {
        CCTV_Create(cctvid);
        CCTV_NOTIFICATION(playerid, 1, "CCTV diaktifkan!");
    }
    else
    {
        CCTV_Destroy(cctvid);
        CCTV_NOTIFICATION(playerid, 1, "CCTV dinonaktifkan!");
    }

    CCTV_Save(cctvid);
    return 1;
}

YCMD:gotocctv(playerid, params[], help)
{
    if(!CCTV_ADMIN_CHECK(playerid)) return CCTV_PERMISSION_ERROR(playerid);

    new cctvid;
    if(sscanf(params, "d", cctvid))
        return CCTV_USAGE_MSG(playerid, "/gotocctv [cctvid]");

    if(!Iter_Contains(CCTVs, cctvid))
        return CCTV_ERROR_MSG(playerid, "CCTV ID tidak valid!");

    SetPlayerPos(playerid, CCTVData[cctvid][cctvPos][0], CCTVData[cctvid][cctvPos][1], CCTVData[cctvid][cctvPos][2] - 2.0);
    SetPlayerVirtualWorld(playerid, CCTVData[cctvid][cctvWorld]);
    SetPlayerInterior(playerid, CCTVData[cctvid][cctvInterior]);

    new string[64];
    format(string, sizeof(string), "Teleported to CCTV ID %d!", cctvid);
    CCTV_NOTIFICATION(playerid, 1, string);
    return 1;
}

YCMD:cctvcek(playerid, params[], help)
{
    new string[2048], count = 0;
    strcat(string, "ID\tName\tDistance\n");

    foreach(new cctvid : CCTVs)
    {
        if(CCTVData[cctvid][cctvActive] &&
           GetPlayerVirtualWorld(playerid) == CCTVData[cctvid][cctvWorld] &&
           GetPlayerInterior(playerid) == CCTVData[cctvid][cctvInterior])
        {
            if(CanPlayerAccessCCTV(playerid, cctvid))
            {
                new Float:distance = GetPlayerDistanceFromPoint(playerid, CCTVData[cctvid][cctvPos][0], CCTVData[cctvid][cctvPos][1], CCTVData[cctvid][cctvPos][2]);

                new temp[128];
                format(temp, sizeof(temp), "%d\t%s\t%.1fm\n", cctvid, CCTVData[cctvid][cctvName], distance);
                strcat(string, temp);
                count++;
            }
        }
    }

    if(count == 0)
        strcat(string, "No accessible CCTV cameras found nearby.");

    ShowPlayerDialog(playerid, 9998, DIALOG_STYLE_TABLIST_HEADERS, "Accessible CCTVs", string, "Close", "");
    return 1;
}

YCMD:stopcctv(playerid, params[], help)
{
    if(!CCTVViewer[playerid][isViewingCCTV])
        return CCTV_ERROR_MSG(playerid, "Anda tidak sedang melihat CCTV!");

    CCTV_StopViewing(playerid);
    return 1;
}

hook OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{
    if(newkeys & KEY_YES && GetPlayerState(playerid) == PLAYER_STATE_ONFOOT)
    {
        new cctvid = GetPlayerNearestCCTV(playerid);
        if(cctvid != -1)
        {
            CCTV_StartViewing(playerid, cctvid);
        }
    }

    if(CCTVViewer[playerid][isViewingCCTV])
    {
        new cctvid = CCTVViewer[playerid][viewingCCTVID];

        if(newkeys & KEY_LEFT)
        {
            CCTVViewer[playerid][cctvViewAngle] -= CCTV_ROTATION_SPEED;
            if(CCTVViewer[playerid][cctvViewAngle] < 0.0)
                CCTVViewer[playerid][cctvViewAngle] += 360.0;
            CCTV_UpdateCameraView(playerid, cctvid);
        }
        else if(newkeys & KEY_RIGHT)
        {
            CCTVViewer[playerid][cctvViewAngle] += CCTV_ROTATION_SPEED;
            if(CCTVViewer[playerid][cctvViewAngle] >= 360.0)
                CCTVViewer[playerid][cctvViewAngle] -= 360.0;
            CCTV_UpdateCameraView(playerid, cctvid);
        }

        if(newkeys & KEY_UP)
        {
            CCTVViewer[playerid][cctvViewPitch] += 2.0;
            if(CCTVViewer[playerid][cctvViewPitch] > 20.0)
                CCTVViewer[playerid][cctvViewPitch] = 20.0;
            CCTV_UpdateCameraView(playerid, cctvid);
        }
        else if(newkeys & KEY_DOWN)
        {
            CCTVViewer[playerid][cctvViewPitch] -= 2.0;
            if(CCTVViewer[playerid][cctvViewPitch] < -20.0)
                CCTVViewer[playerid][cctvViewPitch] = -20.0;
            CCTV_UpdateCameraView(playerid, cctvid);
        }

        if(newkeys & KEY_SECONDARY_ATTACK)
        {
            CCTV_StopViewing(playerid);
        }
    }
}

hook OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
    if(CCTVViewer[playerid][isViewingCCTV])
    {
        new cctvid = CCTVViewer[playerid][viewingCCTVID];

        if(playertextid == CCTVViewer[playerid][cctvHUD][2])
        {
            CCTVViewer[playerid][cctvViewAngle] -= CCTV_ROTATION_SPEED;
            if(CCTVViewer[playerid][cctvViewAngle] < 0.0)
                CCTVViewer[playerid][cctvViewAngle] += 360.0;
            CCTV_UpdateCameraView(playerid, cctvid);
        }
        else if(playertextid == CCTVViewer[playerid][cctvHUD][3])
        {
            CCTVViewer[playerid][cctvViewAngle] += CCTV_ROTATION_SPEED;
            if(CCTVViewer[playerid][cctvViewAngle] >= 360.0)
                CCTVViewer[playerid][cctvViewAngle] -= 360.0;
            CCTV_UpdateCameraView(playerid, cctvid);
        }
        else if(playertextid == CCTVViewer[playerid][cctvHUD][4])
        {
            CCTVViewer[playerid][cctvViewPitch] += 2.0;
            if(CCTVViewer[playerid][cctvViewPitch] > 20.0)
                CCTVViewer[playerid][cctvViewPitch] = 20.0;
            CCTV_UpdateCameraView(playerid, cctvid);
        }
        else if(playertextid == CCTVViewer[playerid][cctvHUD][5])
        {
            CCTVViewer[playerid][cctvViewPitch] -= 2.0;
            if(CCTVViewer[playerid][cctvViewPitch] < -20.0)
                CCTVViewer[playerid][cctvViewPitch] = -20.0;
            CCTV_UpdateCameraView(playerid, cctvid);
        }
        else if(playertextid == CCTVViewer[playerid][cctvHUD][6])
        {
            CCTV_StopViewing(playerid);
        }
    }
}

hook OnPlayerEditDynamicObject(playerid, STREAMER_TAG_OBJECT:objectid, response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz)
{
    if(CCTVViewer[playerid][viewingCCTVID] != -1)
    {
        new cctvid = CCTVViewer[playerid][viewingCCTVID];

        if(objectid == CCTVData[cctvid][cctvObject])
        {
            if(response == EDIT_RESPONSE_FINAL)
            {
                CCTVData[cctvid][cctvPos][0] = x;
                CCTVData[cctvid][cctvPos][1] = y;
                CCTVData[cctvid][cctvPos][2] = z;
                CCTVData[cctvid][cctvAngle] = rz;

                if(IsValidDynamic3DTextLabel(CCTVData[cctvid][cctvLabel]))
                {
                    DestroyDynamic3DTextLabel(CCTVData[cctvid][cctvLabel]);

                    new labeltext[128];
                    format(labeltext, sizeof(labeltext), "{00FF00}CCTV: %s\n{FFFFFF}Press {FFFF00}Y {FFFFFF}to access", CCTVData[cctvid][cctvName]);
                    CCTVData[cctvid][cctvLabel] = CreateDynamic3DTextLabel(labeltext, 0xFFFFFFFF,
                        x, y, z + 0.5, 10.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0,
                        CCTVData[cctvid][cctvWorld], CCTVData[cctvid][cctvInterior], -1, 100.0);
                }

                CCTV_Save(cctvid);
                CCTVViewer[playerid][viewingCCTVID] = -1;
                CCTV_NOTIFICATION(playerid, 1, "CCTV position updated successfully!");
            }
            else if(response == EDIT_RESPONSE_CANCEL)
            {
                CCTVViewer[playerid][viewingCCTVID] = -1;
                CCTV_NOTIFICATION(playerid, 1, "CCTV edit cancelled.");
            }
        }
    }
}
