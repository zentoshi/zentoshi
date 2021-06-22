Sample init scripts and service configuration for zenxd
==========================================================

Sample scripts and configuration files for systemd, Upstart and OpenRC
can be found in the contrib/init folder.

    contrib/init/zenxd.service:    systemd service unit configuration
    contrib/init/zenxd.openrc:     OpenRC compatible SysV style init script
    contrib/init/zenxd.openrcconf: OpenRC conf.d file
    contrib/init/zenxd.conf:       Upstart service configuration file
    contrib/init/zenxd.init:       CentOS compatible SysV style init script

Service User
---------------------------------

All three Linux startup configurations assume the existence of a "zenxcore" user
and group.  They must be created before attempting to use these scripts.
The OS X configuration assumes zenxd will be set up for the current user.

Configuration
---------------------------------

At a bare minimum, zenxd requires that the rpcpassword setting be set
when running as a daemon.  If the configuration file does not exist or this
setting is not set, zenxd will shutdown promptly after startup.

This password does not have to be remembered or typed as it is mostly used
as a fixed token that zenxd and client programs read from the configuration
file, however it is recommended that a strong and secure password be used
as this password is security critical to securing the wallet should the
wallet be enabled.

If zenxd is run with the "-server" flag (set by default), and no rpcpassword is set,
it will use a special cookie file for authentication. The cookie is generated with random
content when the daemon starts, and deleted when it exits. Read access to this file
controls who can access it through RPC.

By default the cookie is stored in the data directory, but it's location can be overridden
with the option '-rpccookiefile'.

This allows for running zenxd without having to do any manual configuration.

`conf`, `pid`, and `wallet` accept relative paths which are interpreted as
relative to the data directory. `wallet` *only* supports relative paths.

For an example configuration file that describes the configuration settings,
see `contrib/debian/examples/zenx.conf`.

Paths
---------------------------------

### Linux

All three configurations assume several paths that might need to be adjusted.

Binary:              `/usr/bin/zenxd`  
Configuration file:  `/etc/zenxcore/zenx.conf`  
Data directory:      `/var/lib/zenxd`  
PID file:            `/var/run/zenxd/zenxd.pid` (OpenRC and Upstart) or `/var/lib/zenxd/zenxd.pid` (systemd)  
Lock file:           `/var/lock/subsys/zenxd` (CentOS)  

The configuration file, PID directory (if applicable) and data directory
should all be owned by the zenxcore user and group.  It is advised for security
reasons to make the configuration file and data directory only readable by the
zenxcore user and group.  Access to zenx-cli and other zenxd rpc clients
can then be controlled by group membership.

### Mac OS X

Binary:              `/usr/local/bin/zenxd`  
Configuration file:  `~/Library/Application Support/ZenXCore/zenx.conf`  
Data directory:      `~/Library/Application Support/ZenXCore`  
Lock file:           `~/Library/Application Support/ZenXCore/.lock`  

Installing Service Configuration
-----------------------------------

### systemd

Installing this .service file consists of just copying it to
/usr/lib/systemd/system directory, followed by the command
`systemctl daemon-reload` in order to update running systemd configuration.

To test, run `systemctl start zenxd` and to enable for system startup run
`systemctl enable zenxd`

### OpenRC

Rename zenxd.openrc to zenxd and drop it in /etc/init.d.  Double
check ownership and permissions and make it executable.  Test it with
`/etc/init.d/zenxd start` and configure it to run on startup with
`rc-update add zenxd`

### Upstart (for Debian/Ubuntu based distributions)

Drop zenxd.conf in /etc/init.  Test by running `service zenxd start`
it will automatically start on reboot.

NOTE: This script is incompatible with CentOS 5 and Amazon Linux 2014 as they
use old versions of Upstart and do not supply the start-stop-daemon utility.

### CentOS

Copy zenxd.init to /etc/init.d/zenxd. Test by running `service zenxd start`.

Using this script, you can adjust the path and flags to the zenxd program by
setting the ZENXD and FLAGS environment variables in the file
/etc/sysconfig/zenxd. You can also use the DAEMONOPTS environment variable here.

### Mac OS X

Copy org.zenx.zenxd.plist into ~/Library/LaunchAgents. Load the launch agent by
running `launchctl load ~/Library/LaunchAgents/org.zenx.zenxd.plist`.

This Launch Agent will cause zenxd to start whenever the user logs in.

NOTE: This approach is intended for those wanting to run zenxd as the current user.
You will need to modify org.zenx.zenxd.plist if you intend to use it as a
Launch Daemon with a dedicated zenxcore user.

Auto-respawn
-----------------------------------

Auto respawning is currently only configured for Upstart and systemd.
Reasonable defaults have been chosen but YMMV.
