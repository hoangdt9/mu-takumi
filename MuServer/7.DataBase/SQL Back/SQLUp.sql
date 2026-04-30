ALTER TABLE [Character] ADD DanhHieu int NOT NULL DEFAULT (0)
ALTER TABLE [Character] ADD LucChien int NOT NULL DEFAULT (0)
ALTER TABLE [Character] ADD TimeReset smalldatetime NULL DEFAULT (0)

/****** Object:  Table [dbo].[DataNapGame]    Script Date: 10/24/2024 23:59:08 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

SET ANSI_PADDING ON
GO

CREATE TABLE [dbo].[DataNapGame](
	[STT] [int] IDENTITY(1,1) NOT NULL,
	[Account] [varchar](11) NULL,
	[Name] [varchar](11) NULL,
	[TienNap] [int] NOT NULL,
	[Checking] [int] NULL,
	[Status] [int] NOT NULL
) ON [PRIMARY]

GO

SET ANSI_PADDING OFF
GO

ALTER TABLE [dbo].[DataNapGame] ADD  DEFAULT ((0)) FOR [TienNap]
GO

ALTER TABLE [dbo].[DataNapGame] ADD  DEFAULT ((0)) FOR [Status]
GO



/****** Object:  Table [dbo].[ItemMarketData]    Script Date: 10/24/2024 23:59:30 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

SET ANSI_PADDING ON
GO

CREATE TABLE [dbo].[ItemMarketData](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[Account] [varchar](10) NULL,
	[Name] [varchar](10) NULL,
	[PriceType] [int] NOT NULL,
	[PriceValue] [int] NOT NULL,
	[Status] [int] NOT NULL,
	[TypeItem] [int] NOT NULL,
	[Date] [varchar](20) NULL,
	[Item] [varbinary](16) NULL,
	[Time] [int] NULL,
	[Pass] [int] NULL
) ON [PRIMARY]

GO

SET ANSI_PADDING OFF
GO

ALTER TABLE [dbo].[ItemMarketData] ADD  DEFAULT ((0)) FOR [PriceType]
GO

ALTER TABLE [dbo].[ItemMarketData] ADD  DEFAULT ((0)) FOR [PriceValue]
GO

ALTER TABLE [dbo].[ItemMarketData] ADD  DEFAULT ((0)) FOR [Status]
GO

ALTER TABLE [dbo].[ItemMarketData] ADD  DEFAULT ((0)) FOR [TypeItem]
GO



/****** Object:  Table [dbo].[CustomItemBank]    Script Date: 10/25/2024 01:01:15 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

SET ANSI_PADDING ON
GO

CREATE TABLE [dbo].[CustomItemBank](
	[AccountID] [varchar](10) NOT NULL,
	[ItemIndex] [int] NOT NULL,
	[ItemLevel] [int] NOT NULL,
	[ItemCount] [int] NOT NULL,
	[AutoPick] [int] NOT NULL
) ON [PRIMARY]

GO

SET ANSI_PADDING OFF
GO

ALTER TABLE [dbo].[CustomItemBank] ADD  CONSTRAINT [ItemIndex]  DEFAULT ((0)) FOR [ItemIndex]
GO

ALTER TABLE [dbo].[CustomItemBank] ADD  CONSTRAINT [ItemLevel]  DEFAULT ((0)) FOR [ItemLevel]
GO

ALTER TABLE [dbo].[CustomItemBank] ADD  CONSTRAINT [ItemCount]  DEFAULT ((0)) FOR [ItemCount]
GO

ALTER TABLE [dbo].[CustomItemBank] ADD  DEFAULT ((0)) FOR [AutoPick]



/****** Object:  Table [dbo].[EquipInventory]    Script Date: 11/01/2024 21:16:35 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

SET ANSI_PADDING ON
GO

CREATE TABLE [dbo].[EquipInventory](
	[CharName] [varchar](10) NOT NULL,
	[Items] [varbinary](3840) NULL
) ON [PRIMARY]

GO

SET ANSI_PADDING OFF
GO


ALTER TABLE [Character] ADD mLvVIPChar int NOT NULL DEFAULT (0)
ALTER TABLE [Character] ADD mPointVIPChar int NOT NULL DEFAULT (0)