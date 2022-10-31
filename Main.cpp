# include <Siv3D.hpp> // OpenSiv3D v0.6.5
# include "Multiplayer_Photon.hpp"
# include "PHOTON_APP_ID.SECRET"

struct SinglePixel {
	Point pos;
	Color color;
};

struct PixelData {
	int32 size = 8;
	int32 width = 320;
	int32 height = 180;
	Grid<SinglePixel> data;
};

// ユーザ定義型
struct PixelSerialize
{
	SinglePixel pixel;

	// シリアライズに対応させるためのメンバ関数を定義する
	template <class Archive>
	void SIV3D_SERIALIZE(Archive& archive)
	{
		archive(pixel);
	}
};



class MyNetwork : public Multiplayer_Photon
{
public:

	static constexpr int32 MaxPlayers = 3;

	using Multiplayer_Photon::Multiplayer_Photon;

private:

	Array<LocalPlayer> m_localPlayers;

	void connectReturn([[maybe_unused]] const int32 errorCode, const String& errorString, const String& region, [[maybe_unused]] const String& cluster) override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::connectReturn() [サーバへの接続を試みた結果を処理する]";
		}

		if (errorCode)
		{
			if (m_verbose)
			{
				Print << U"[サーバへの接続に失敗] " << errorString;
			}

			return;
		}

		if (m_verbose)
		{
			Print << U"[サーバへの接続に成功]";
			Print << U"[region: {}]"_fmt(region);
			Print << U"[ユーザ名: {}]"_fmt(getUserName());
			Print << U"[ユーザ ID: {}]"_fmt(getUserID());
		}

		Scene::SetBackground(ColorF{ 0.4, 0.5, 0.6 });
	}

	void disconnectReturn() override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::disconnectReturn() [サーバから切断したときに呼ばれる]";
		}

		m_localPlayers.clear();

		Scene::SetBackground(Palette::DefaultBackground);
	}

	void joinRandomRoomReturn([[maybe_unused]] const LocalPlayerID playerID, const int32 errorCode, const String& errorString) override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::joinRandomRoomReturn() [既存のランダムなルームに参加を試みた結果を処理する]";
		}

		if (errorCode == NoRandomMatchFound)
		{
			const RoomName roomName = (getUserName() + U"'s room-" + ToHex(RandomUint32()));

			if (m_verbose)
			{
				Print << U"[参加可能なランダムなルームが見つからなかった]";
				Print << U"[自分でルーム " << roomName << U" を新規作成する]";
			}

			createRoom(roomName, MaxPlayers);

			return;
		}
		else if (errorCode)
		{
			if (m_verbose)
			{
				Print << U"[既存のランダムなルームへの参加でエラーが発生] " << errorString;
			}

			return;
		}

		if (m_verbose)
		{
			Print << U"[既存のランダムなルームに参加できた]";
		}
	}

	void createRoomReturn([[maybe_unused]] const LocalPlayerID playerID, const int32 errorCode, const String& errorString) override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::createRoomReturn() [ルームを新規作成した結果を処理する]";
		}

		if (errorCode)
		{
			if (m_verbose)
			{
				Print << U"[ルームの新規作成でエラーが発生] " << errorString;
			}

			return;
		}

		if (m_verbose)
		{
			Print << U"[ルーム " << getCurrentRoomName() << U" の作成に成功]";
		}
	}

	void joinRoomEventAction(const LocalPlayer& newPlayer, [[maybe_unused]] const Array<LocalPlayerID>& playerIDs, const bool isSelf) override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::joinRoomEventAction() [誰か（自分を含む）が現在のルームに参加したときに呼ばれる]";
		}

		if (m_verbose)
		{
			Print << U"[{} (ID: {}) がルームに参加した。ローカル ID: {}] {}"_fmt(newPlayer.userName, newPlayer.userID, newPlayer.localID, (isSelf ? U"(自分自身)" : U""));

			m_localPlayers = getLocalPlayers();

			Print << U"現在の " << getCurrentRoomName() << U" のルームメンバー";

			for (const auto& player : m_localPlayers)
			{
				Print << U"- [{}] {} (id: {}) {}"_fmt(player.localID, player.userName, player.userID, player.isHost ? U"(host)" : U"");
			}
		}
	}

	void leaveRoomEventAction(const LocalPlayerID playerID, [[maybe_unused]] const bool isInactive) override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::joinRoomEventAction() [誰かがルームから退出したら呼ばれる]";
		}

		if (m_verbose)
		{
			for (const auto& player : m_localPlayers)
			{
				if (player.localID == playerID)
				{
					Print << U"[{} (ID: {}, ローカル ID: {}) がルームから退出した]"_fmt(player.userName, player.userID, player.localID);
				}
			}

			m_localPlayers = getLocalPlayers();

			Print << U"現在の " << getCurrentRoomName() << U" のルームメンバー";

			for (const auto& player : m_localPlayers)
			{
				Print << U"- [{}] {} (ID: {}) {}"_fmt(player.localID, player.userName, player.userID, player.isHost ? U"(host)" : U"");
			}
		}
	}

	void leaveRoomReturn(int32 errorCode, const String& errorString) override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::leaveRoomReturn() [ルームから退出したときに呼ばれる]";
		}

		m_localPlayers.clear();

		if (errorCode)
		{
			if (m_verbose)
			{
				Print << U"[ルームからの退出でエラーが発生] " << errorString;
			}

			return;
		}
	}
	/*
	void customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const int32 data) override
	{
		Print << U"<<< [" << playerID << U"] からの eventCode: " << eventCode << U", data: int32(" << data << U") を受信";
	}

	void customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const String& data) override
	{
		Print << U"<<< [" << playerID << U"] からの eventCode: " << eventCode << U", data: String(" << data << U") を受信";
	}

	void customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Point& data) override
	{
		Print << U"<<< [" << playerID << U"] からの eventCode: " << eventCode << U", data: Point" << data << U" を受信";
	}

	void customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Array<int32>& data) override
	{
		Print << U"<<< [" << playerID << U"] からの eventCode: " << eventCode << U", data: Array<int32>" << data << U" を受信";
	}

	void customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Array<String>& data) override
	{
		Print << U"<<< [" << playerID << U"] からの eventCode: " << eventCode << U", data: Array<String>" << data << U" を受信";
	}
	*/
	// シリアライズデータを受信したときに呼ばれる関数をオーバーライドしてカスタマイズする
	void customEventAction(const LocalPlayerID playerID, const uint8 eventCode, Deserializer<MemoryViewReader>& reader) override
	{
		if (eventCode == 123)
		{
			PixelSerialize pixel;
			reader(pixel);
			//Print << U"<<< [" << playerID << U"] からの MyData(" << pixel.word << U", " << mydata.pos << U") を受信";
			
		}
	}
};

void Main()
{
	Window::Resize(1280, 720);
	const std::string secretAppID{ SIV3D_OBFUSCATE(PHOTON_APP_ID) };
	MyNetwork network{ secretAppID, U"1.0", Verbose::Yes };
	TextEditState userNameInBox;
	userNameInBox.text = U"user";
	PixelData pixelData;
	while (System::Update())
	{
		network.update();

		SimpleGUI::TextBox(userNameInBox, Vec2{ 800,20 }, 160, 16, true);

		if (SimpleGUI::Button(U"Connect", Vec2{ 1000, 20 }, 160, (not network.isActive())))
		{
			const String userName = userNameInBox.text;
			network.connect(userName);
		}

		if (SimpleGUI::Button(U"Disconnect", Vec2{ 1000, 60 }, 160, network.isActive()))
		{
			network.disconnect();
		}

		if (SimpleGUI::Button(U"Join Room", Vec2{ 1000, 100 }, 160, network.isInLobby()))
		{
			network.joinRandomRoom(MyNetwork::MaxPlayers);
		}

		if (SimpleGUI::Button(U"Leave Room", Vec2{ 1000, 140 }, 160, network.isInRoom()))
		{
			network.leaveRoom();
		}
		/*
		if (SimpleGUI::Button(U"Send int32", Vec2{ 1000, 180 }, 200, network.isInRoom()))
		{
			const int32 n = Random(0, 10000);
			Print << U"eventCode: 0, int32(" << n << U") を送信 >>>";
			network.sendEvent(0, n);
		}

		if (SimpleGUI::Button(U"Send String", Vec2{ 1000, 220 }, 200, network.isInRoom()))
		{
			const String s = Sample({ U"Hello!", U"Thank you!", U"Nice!" });
			Print << U"eventCode: 0, String(" << s << U") を送信 >>>";
			network.sendEvent(0, s);
		}

		if (SimpleGUI::Button(U"Send Point", Vec2{ 1000, 260 }, 200, network.isInRoom()))
		{
			const Point pos = RandomPoint(Scene::Rect());
			Print << U"eventCode: 0, Point" << pos << U" を送信 >>>";
			network.sendEvent(0, pos);
		}

		if (SimpleGUI::Button(U"Send Array<int32>", Vec2{ 1000, 300 }, 200, network.isInRoom()))
		{
			Array<int32> v(3);
			for (auto& n : v)
			{
				n = Random(0, 1000);
			}
			Print << U"eventCode: 0, Array<int32>" << v << U" を送信 >>>";
			network.sendEvent(0, v);
		}

		if (SimpleGUI::Button(U"Send Array<String>", Vec2{ 1000, 340 }, 200, network.isInRoom()))
		{
			Array<String> words(3);
			for (auto& word : words)
			{
				word = Sample({ U"apple", U"bird", U"cat", U"dog" });
			}
			Print << U"eventCode: 0, Array<String>" << words << U" を送信 >>>";
			network.sendEvent(0, words);
		}

		// ランダムな MyData を送るボタン
		if (SimpleGUI::Button(U"Send MyData", Vec2{ 1000, 380 }, 200, network.isInRoom()))
		{
			MyData myData;
			myData.word = Sample({ U"apple", U"bird", U"cat", U"dog" });
			myData.pos = RandomPoint(Scene::Rect());

			Print << U"eventCode: 123, MyData(" << myData.word << U", " << myData.pos << U") を送信 >>>";
			network.sendEvent(123, Serializer<MemoryWriter>{}(myData));
		}
		*/


	}
}
