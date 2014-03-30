
-- Table: sensor
CREATE TABLE sensor ( 
    sensor_id   INTEGER PRIMARY KEY
                        NOT NULL,
    room_id     INTEGER NOT NULL
                        REFERENCES room ( room_id ),
    sensor_type INTEGER NOT NULL,
    [desc]      VARCHAR 
);

INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (101, 1, 1, 'TEMP');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (102, 1, 1, 'TEMP');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (103, 1, 1, 'TEMP');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (104, 1, 2, 'LIGHT');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (105, 1, 2, 'LIGHT');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (106, 1, 2, 'LIGHT');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (107, 1, 3, 'HUMI');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (108, 1, 3, 'HUMI');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (109, 1, 3, 'HUMI');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (110, 1, 4, 'CO2');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (111, 1, 4, 'CO2');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (112, 1, 4, 'CO2');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (113, 2, 1, 'TEMP');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (114, 2, 1, 'TEMP');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (115, 2, 1, 'TEMP');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (116, 2, 2, 'LIGHT');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (117, 2, 2, 'LIGHT');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (118, 2, 2, 'LIGHT');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (119, 2, 3, 'HUMI');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (120, 2, 3, 'HUMI');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (121, 2, 3, 'HUMI');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (122, 2, 4, 'CO2');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (123, 2, 4, 'CO2');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (124, 2, 4, 'CO2');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (125, 3, 1, 'TEMP');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (126, 3, 1, 'TEMP');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (127, 3, 1, 'TEMP');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (128, 3, 2, 'LIGHT');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (129, 3, 2, 'LIGHT');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (130, 3, 2, 'LIGHT');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (131, 3, 3, 'HUMI');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (132, 3, 3, 'HUMI');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (133, 3, 3, 'HUMI');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (134, 3, 4, 'CO2');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (135, 3, 4, 'CO2');
INSERT INTO [sensor] ([sensor_id], [room_id], [sensor_type], [desc]) VALUES (136, 3, 4, 'CO2');
