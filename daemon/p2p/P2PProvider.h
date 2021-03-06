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
#pragma once
#include <discovery/DiscoveryService.h>
#include <util/network.h>
#include <util/log_scope.h>
#include <set>
#include <mutex>

namespace librevault {

class WSServer;
class WSClient;

class PortMappingService;
class NodeKey;
class FolderService;

class P2PProvider {
	friend class ControlServer;
	LOG_SCOPE("P2PProvider");
public:
	P2PProvider(NodeKey& node_key, PortMappingService& port_mapping, FolderService& folder_service);
	virtual ~P2PProvider();

	void run() {ios_.start(1);}
	void stop() {ios_.stop();}

	void add_node(DiscoveryService::ConnectCredentials node_cred, std::shared_ptr<FolderGroup> group_ptr);

	/* Loopback detection */
	void mark_loopback(const tcp_endpoint& endpoint);
	bool is_loopback(const tcp_endpoint& endpoint);
	bool is_loopback(const blob& pubkey);

private:
	multi_io_service ios_;
	NodeKey& node_key_;

	/* WebSocket sockets */
	std::unique_ptr<WSServer> ws_server_;
	std::unique_ptr<WSClient> ws_client_;

	/* Loopback detection */
	std::set<tcp_endpoint> loopback_blacklist_;
	std::mutex loopback_blacklist_mtx_;
};

} /* namespace librevault */
