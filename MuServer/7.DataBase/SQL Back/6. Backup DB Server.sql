
USE [MuOnline_TakumiUP15]
GO

DBCC SHRINKDATABASE(N'MuOnline_TakumiUP15')
 
Declare @dateBackup Varchar(100)
Set @dateBackup = 'E:\MuOnline\Louis13\ProjectTanNat\MuServer\DB\MuOnlineDB ' +
Convert(varchar, datepart( HOUR , Getdate() )) + ' Gio'+
Convert(varchar, datepart( MINUTE , Getdate() )) + ' Phut '+
Convert(varchar, datepart( SECOND , Getdate() )) + ' Giay '+
Convert(varchar, datepart( day , Getdate() )) + '-'+
Convert( varchar , datepart( month , Getdate() ) ) + '-'+
Convert( varchar, datepart( year , Getdate() ) ) + '.bak'

BACKUP DATABASE [MuOnline_TakumiUP15] TO DISK = @dateBackup WITH NOFORMAT, NOINIT, NAME = N'MuOnline_TakumiUP15-Full Database Backup', SKIP, NOREWIND, NOUNLOAD, STATS = 10
GO