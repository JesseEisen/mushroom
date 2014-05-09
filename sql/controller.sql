
-- Table: controller
CREATE TABLE controller ( 
    controller_id   INTEGER PRIMARY KEY
                            NOT NULL,
    room_id         INTEGER NOT NULL
                            REFERENCES room ( room_id ),
    controller_type INTEGER NOT NULL,
    [desc]          VARCHAR 
);

INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (1, 1, 1, 'XUNHUAN_FAN');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (2, 1, 2, 'JINFENG_FAN');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (3, 1, 3, 'PAIFENG_FAN');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (4, 1, 4, 'JIASHIQI');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (5, 1, 5, 'YASUOJI');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (6, 1, 6, 'NEIJI');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (7, 1, 7, 'YELLO_LIGHT');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (8, 1, 8, 'RED_LIGHT');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (9, 1, 9, 'BLUE_LIGHT');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (10, 2, 1, 'XUNHUAN_FAN');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (11, 2, 2, 'JINFENG_FAN');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (12, 2, 3, 'PAIFENG_FAN');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (13, 2, 4, 'JIASHIQI');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (14, 2, 5, 'YASUOJI');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (15, 2, 6, 'NEIJI');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (16, 2, 7, 'YELLO_LIGHT');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (17, 2, 8, 'RED_LIGHT');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (18, 2, 9, 'BLUE_LIGHT');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (19, 3, 1, 'XUNHUAN_FAN');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (20, 3, 2, 'JINFENG_FAN');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (21, 3, 3, 'PAIFENG_FAN');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (22, 3, 4, 'JIASHIQI');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (23, 3, 5, 'YASUOJI');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (24, 3, 6, 'NEIJI');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (25, 3, 7, 'YELLO_LIGHT');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (26, 3, 8, 'RED_LIGHT');
INSERT INTO [controller] ([controller_id], [room_id], [controller_type], [desc]) VALUES (27, 3, 9, 'BLUE_LIGHT');
