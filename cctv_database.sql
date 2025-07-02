-- ================================================================================
-- SA-MP CCTV SURVEILLANCE SYSTEM v2.0 - DATABASE STRUCTURE
-- ================================================================================
-- Author: LeoXD SAMP++
-- Compatible: MySQL 5.7+ / MariaDB 10.0+
-- License: MIT License
-- ================================================================================

-- Create CCTV data table
CREATE TABLE IF NOT EXISTS `cctv_data` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `PosX` float NOT NULL DEFAULT '0',
  `PosY` float NOT NULL DEFAULT '0',
  `PosZ` float NOT NULL DEFAULT '0',
  `Angle` float NOT NULL DEFAULT '0',
  `World` int(11) NOT NULL DEFAULT '0',
  `Interior` int(11) NOT NULL DEFAULT '0',
  `OwnerType` int(11) NOT NULL DEFAULT '0' COMMENT '0=Admin, 1=Police, 2=Faction, 3=House',
  `OwnerID` int(11) NOT NULL DEFAULT '0' COMMENT 'FactionID or HouseID',
  `Name` varchar(64) NOT NULL DEFAULT 'CCTV Camera',
  `Active` int(11) NOT NULL DEFAULT '1' COMMENT '0=Inactive, 1=Active',
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ================================================================================
-- FIELD DESCRIPTIONS:
-- ================================================================================
-- ID          : Unique CCTV identifier (Auto increment)
-- PosX        : X coordinate position of CCTV camera
-- PosY        : Y coordinate position of CCTV camera  
-- PosZ        : Z coordinate position of CCTV camera
-- Angle       : Facing angle of CCTV camera (0-360 degrees)
-- World       : Virtual world ID where CCTV is located
-- Interior    : Interior ID where CCTV is located
-- OwnerType   : Type of owner (0=Admin, 1=Police, 2=Faction, 3=House)
-- OwnerID     : ID of the owner (FactionID for factions, HouseID for houses)
-- Name        : Display name of the CCTV camera
-- Active      : Status of CCTV (0=Inactive/Disabled, 1=Active/Enabled)
-- ================================================================================

-- Sample data (optional - remove if not needed)
INSERT INTO `cctv_data` (`ID`, `PosX`, `PosY`, `PosZ`, `Angle`, `World`, `Interior`, `OwnerType`, `OwnerID`, `Name`, `Active`) VALUES
(1, 1544.8, -1675.5, 16.2, 90.0, 0, 0, 0, 0, 'City Hall Main Entrance', 1),
(2, 1481.0, -1771.2, 21.5, 180.0, 0, 0, 1, 0, 'Police Station Front', 1),
(3, 2247.5, -1665.8, 18.3, 270.0, 0, 0, 0, 0, 'Hospital Emergency', 1);

-- ================================================================================
-- INSTALLATION NOTES:
-- ================================================================================
-- 1. Execute this SQL file in your MySQL database
-- 2. Make sure your gamemode has MySQL connection established
-- 3. Update the CCTV_MYSQL_CONNECTION define in cctv_system.inc
-- 4. Call CCTV_Load() in your OnGameModeInit callback
-- 5. Test with /addcctv command in-game
-- ================================================================================
