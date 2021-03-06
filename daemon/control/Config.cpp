/* Copyright (C) 2016 Alexander Shishenko <alex@shishenko.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
#include "Config.h"
#include <codecvt>
#include "util/file_util.h"
#include <boost/asio/ip/host_name.hpp>

namespace librevault {

Config::Config(boost::filesystem::path appdata_path) {
	if(appdata_path.empty())
		paths_.appdata_path = default_appdata_path();
	else
		paths_.appdata_path = std::move(appdata_path);

	boost::filesystem::create_directories(paths_.appdata_path);

	paths_.client_config_path = paths_.appdata_path / "globals.json";
	paths_.folders_config_path = paths_.appdata_path / "folders.json";
	paths_.log_path = paths_.appdata_path / "librevault.log";
	paths_.key_path = paths_.appdata_path / "key.pem";
	paths_.cert_path = paths_.appdata_path / "cert.pem";
	paths_.dht_session_path = paths_.appdata_path / "mldht_session.bin";

	make_defaults();
	load();

	config_changed.connect([this](){save();});
}

Config::~Config() {
	save();
}

std::unique_ptr<Config> Config::instance_ = nullptr;

void Config::set_globals(Json::Value globals_conf) {
	globals_custom_ = make_merged(globals_conf, globals_custom_);
	make_merged_globals();
	config_changed();
}

void Config::set_folders(Json::Value folders_conf) {
	folders_custom_ = std::move(folders_conf);
	make_merged_folders();
	config_changed();
}

void Config::make_defaults() {
	/* globals_defaults_ */
	globals_defaults_.clear();
	globals_defaults_["client_name"] = boost::asio::ip::host_name();
	globals_defaults_["control_listen"] = "[::1]:42346";
	globals_defaults_["p2p_listen"] = "[::]:42345";
	globals_defaults_["p2p_download_slots"] = 10;
	globals_defaults_["p2p_request_timeout"] = 10;
	globals_defaults_["p2p_block_size"] = 32768;
	globals_defaults_["natpmp_enabled"] = true;
	globals_defaults_["natpmp_lifetime"] = 3600;
	globals_defaults_["upnp_enabled"] = true;
	globals_defaults_["predef_repeat_interval"] = 30;
	globals_defaults_["multicast4_enabled"] = true;
	globals_defaults_["multicast4_group"] = "239.192.152.144:28914";
	globals_defaults_["multicast4_repeat_interval"] = 30;
	globals_defaults_["multicast6_enabled"] = true;
	globals_defaults_["multicast6_group"] = "[ff08::BD02]:28914";
	globals_defaults_["multicast6_repeat_interval"] = 30;
	globals_defaults_["bttracker_enabled"] = true;
	globals_defaults_["bttracker_num_want"] = 30;
	globals_defaults_["bttracker_min_interval"] = 15;
	globals_defaults_["bttracker_azureus_id"] = "-LV0001-";
	globals_defaults_["bttracker_reconnect_interval"] = 30;
	globals_defaults_["bttracker_packet_timeout"] = 10;
	globals_defaults_["mainline_dht_enabled"] = true;
	globals_defaults_["mainline_dht_port"] = 42347;

	//globals_defaults_["mainline_dht_routers"].append("discovery-mldht.librevault.com:6881");    // TODO: Soon, guys!
	globals_defaults_["mainline_dht_routers"].append("router.utorrent.com:6881");
	globals_defaults_["mainline_dht_routers"].append("router.bittorrent.com:6881");
	globals_defaults_["mainline_dht_routers"].append("dht.transmissionbt.com:6881");
	globals_defaults_["mainline_dht_routers"].append("router.bitcomet.com:6881");
	globals_defaults_["mainline_dht_routers"].append("dht.aelitis.com:6881");

	//globals_defaults_["bttracker_discovery_trackers"].append("udp://discovery-bt.librevault.com:42340");  // TODO: Really soon, guys!
	globals_defaults_["bttracker_trackers"].append("udp://tracker.openbittorrent.com:80");
	globals_defaults_["bttracker_trackers"].append("udp://open.demonii.com:1337");
	globals_defaults_["bttracker_trackers"].append("udp://tracker.coppersurfer.tk:6969");
	globals_defaults_["bttracker_trackers"].append("udp://tracker.leechers-paradise.org:6969");
	globals_defaults_["bttracker_trackers"].append("udp://tracker.opentrackr.org:1337");

	/* folders_defaults_ */
	folders_defaults_.clear();
	folders_defaults_["index_event_timeout"] = 1000;
	folders_defaults_["preserve_unix_attrib"] = false;
	folders_defaults_["preserve_windows_attrib"] = false;
	folders_defaults_["preserve_symlinks"] = false;
	folders_defaults_["normalize_unicode"] = true;
	folders_defaults_["chunk_strong_hash_type"] = 0;
	folders_defaults_["full_rescan_interval"] = 600;
	folders_defaults_["archive_type"] = "trash";
	folders_defaults_["archive_trash_ttl"] = 30;
	folders_defaults_["archive_timestamp_count"] = 5;
	folders_defaults_["mainline_dht_enabled"] = true;
}

Json::Value Config::make_merged(const Json::Value& custom_value, const Json::Value& default_value) {
	Json::Value merged;
	for(auto& name : default_value.getMemberNames())
		merged[name] = custom_value.get(name, default_value[name]);
	for(auto& name : custom_value.getMemberNames())
		if(!merged.isMember(name))
			merged[name] = custom_value[name];
	return merged;
}

void Config::make_merged_globals() {
	globals_ = make_merged(globals_custom_, globals_defaults_);
}

void Config::make_merged_folders() {
	folders_.clear();
	for(auto& folder : folders_custom_)
		folders_.append(make_merged(folder, folders_defaults_));
}

void Config::load() {
	file_wrapper globals_f(paths_.client_config_path, "rb");
	file_wrapper folders_f(paths_.folders_config_path, "rb");

	Json::Reader r;
	r.parse(globals_f.ios(), globals_custom_);
	r.parse(folders_f.ios(), folders_custom_);

	set_globals(globals_custom_);
	set_folders(folders_custom_);
}

void Config::save() {
	file_wrapper globals_f(paths_.client_config_path, "wb");
	file_wrapper folders_f(paths_.folders_config_path, "wb");

	globals_f.ios() << globals_custom_.toStyledString();
	folders_f.ios() << folders_custom_.toStyledString();
}

} /* namespace librevault */
