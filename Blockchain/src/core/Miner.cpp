#include "miner.hpp"


namespace Core
{
	Miner::Miner(HttpServer* server, std::vector<int>& peers, BlockChain& blockchain)
		:
		server(server),
		peers(peers),
		blockchain(blockchain)
	{
		initConsole();
		start(server, blockchain, peers);
		setUpPeer(server, peers, blockchain);
	}

	void Miner::initConsole()
	{
		CreateNewConsole(1024);
	}


	bool Miner::CreateNewConsole(int16_t length)
	{
		bool result = false;
		ReleaseConsole();
		if (AllocConsole())
		{
			AdjustConsoleBuffer(length);
			result = RedirectConsoleIO();
		}
		return result;
	}

	bool Miner::RedirectConsoleIO()
	{
		bool result = true;
		FILE* fp;

		// Redirect STDIN if the console has an input handle
		if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE)
			if (freopen_s(&fp, "CONIN$", "r", stdin) != 0)
				result = false;
			else
				setvbuf(stdin, NULL, _IONBF, 0);

		// Redirect STDOUT if the console has an output handle
		if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE)
			if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0)
				result = false;
			else
				setvbuf(stdout, NULL, _IONBF, 0);

		// Redirect STDERR if the console has an error handle
		if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE)
			if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0)
				result = false;
			else
				setvbuf(stderr, NULL, _IONBF, 0);

		// Make C++ standard streams point to console as well.
		std::ios::sync_with_stdio(true);

		// Clear the error state for each of the C++ standard streams.
		std::wcout.clear();
		std::cout.clear();
		std::wcerr.clear();
		std::cerr.clear();
		std::wcin.clear();
		std::cin.clear();

		return result;
	}

	[[nodiscard]]
	bool Miner::ReleaseConsole()
	{
		bool result = true;
		FILE* fp;
		if (freopen_s(&fp, "NUL:", "r", stdin) != 0)
			result = false;
		else
			setvbuf(stdin, NULL, _IONBF, 0);
		if (freopen_s(&fp, "NUL:", "w", stdout) != 0)
			result = false;
		else
			setvbuf(stdout, NULL, _IONBF, 0);
		if (freopen_s(&fp, "NUL:", "w", stderr) != 0)
			result = false;
		else
			setvbuf(stderr, NULL, _IONBF, 0);
		if (!FreeConsole())
			result = false;

		return result;
	}

	void Miner::AdjustConsoleBuffer(int16_t minLength)
	{
		CONSOLE_SCREEN_BUFFER_INFO conInfo;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
		if (conInfo.dwSize.Y < minLength)
			conInfo.dwSize.Y = minLength;
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), conInfo.dwSize);
	}


	void Miner::process_input(HWND handle, std::vector<int>& peers, BlockChain& bc)
	{
		for (;;)
		{
			int lvl;
			std::string miner;
			//temporary for testing
			//The Transaction Ins/Outs with digital sign(EC) is TODO
			std::string in;
			std::string out;
			float amount;
			std::string input;
			//TRANSACTION TODO HERE
			std::vector<std::string> transaction;
			std::cout << "Type /print to print all blocks" << std::endl;
			std::cout << "Type /add to add transaction" << std::endl;
			std::cout << "Type [name] of the block to look at:" << std::endl;
			std::cin >> input;

			if (input == "/print")
			{
				bc.printBlocks();
			}
			else if (input == "/add")
			{
				spdlog::info("Provide miner's name:");
				std::cin >> miner;
				spdlog::info("inTX:");
				std::cin >> in;
				spdlog::info("outTX:");
				std::cin >> out;
				spdlog::info("Amount:");
				std::cin >> amount;

				//idiot, no time to think, have to get the work done!
				std::string tx = "Miner: " + miner + ", " + "InTX: " + in + ", " + "OutTX: " + out + "Amount: +" + std::to_string(amount) + "WBT";
				transaction.push_back(tx);
				std::cout << "Enter difficulty level:" << std::endl;
				std::cin >> lvl;

				std::pair<std::string, std::string> pair = Utils::findHash(lvl, bc.getNumOfBlocks(), bc.getLatestBlockHash(), transaction);

				//This gonna be the main transaction and digital wallet option for the future logic
				//now it just mimics + .25 WBT each 5 blocks.
				//Bitcoin does this every 210k blocks as far as i know
				if ((bc.getNumOfBlocks() != 0) && ((bc.getNumOfBlocks() % 3) == 0))  MessageBox(handle, "+0.25WBT", "Blockchain Message", MB_OK);
				bc.addBlock(lvl, bc.getNumOfBlocks(), Block::getTime().c_str(), bc.getLatestBlockHash(), pair.first, pair.second, transaction);
				spdlog::info("Updating blockchain\n");
				for (int i = 0; i < peers.size(); i++)
				{
					int port = peers[i];
					printf("--- sending to node %d\n", port);
					HttpClient client("localhost:" + std::to_string(port));
					auto req = client.request("POST", "/updateLedger", bc.serialize());
					//std::cout << "Node " << port << " Response: " << req->content.string() << std::endl;
				}
			}
			else
			{
				for (unsigned i = 0; i < bc.getNumOfBlocks(); i++)
				{
					if (bc.getByName(i) == input)
					{
						bc.printBlock(i);
					}
					else
					{
						spdlog::warn("Either the input is incorrect or i haven't finish this feature yet\n");
						break;
					}
				}
			}
		}
		std::cout << std::endl;
	}


	int Miner::start(HttpServer* server, BlockChain& blockchain, std::vector<int>& peers)
	{
		char in;
		spdlog::info("Test spdlog");
		spdlog::warn("Easy padding in numbers like {:08d}", 12);
		spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
		spdlog::error("Some error message with arg: {}", 1);
		std::cout << "Welcome to WinCoin.\n" << std::endl;
		std::cout << "If you are creating a new blockchain, type 'y',\n if you are joining the existing, type 'j':\n ";
		std::cin >> in;

		server->config.port = getAvilablePort();
		writePort(server->config.port);
		using json = nlohmann::json;
		if (in == 'y') blockchain.behave(BlockChain::Stage::GENESIS);

		else if (in == 'j')
		{
			peers = readPort("file.txt");
			blockchain.behave(BlockChain::Stage::JOIN);

			json j;
			j["port"] = server->config.port;
			for (int i = 0; i < peers.size() - 1; i++)
			{
				int port = peers[i];
				HttpClient client("localhost:" + std::to_string(port));
				std::shared_ptr<HttpClient::Response> response = client.request("POST", "/peerpush", j.dump());
				std::shared_ptr<HttpClient::Response> Defaultresponse = client.request("GET");
			}

			std::vector<std::string> vect;
			for (int a = 0; a < peers.size() - 1; a++)
			{
				int port = peers[a];
				HttpClient client("localhost:" + std::to_string(port));
				std::shared_ptr<HttpClient::Response> response = client.request("GET", "/current");
				vect.push_back(response->content.string());
			}

			json chain = json::parse(vect[0]);
			int max = 0;
			for (int i = 0; i < vect.size(); i++) {
				json json_data = json::parse(vect[i]);
				if (max < json_data["length"].get<int>())
				{
					max = json_data["length"].get<int>();
					chain = json_data;
				}
			}

			for (int a = 0; a < chain["length"].get<int>(); a++)
			{
				auto block = chain["data"][a];
				std::vector<std::string> data = block["data"].get<std::vector<std::string>>();
				blockchain.addBlock(block["difficulty"], block["counter"], block["minedtime"], block["previousHash"], block["hash"], block["nonce"], data);
			}
		}
		else
		{
			return 0;
		}
	}
}