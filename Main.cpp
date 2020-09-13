#include <Siv3D.hpp>

using App = SceneManager<String>;


//タイトルシーン
class Title : public App::Scene {
private:
	Texture m_texture;
public:
	
	//コンストラクタ
	Title(const InitData& init) 
		:IScene(init) 
		, m_texture(Emoji(U"🐧")) {

		}

	//更新関数
	void update() override {
		//左クリックで
		if (MouseL.down()) {
			//ゲームシーンに遷移
			changeScene(U"Game");
		}
	}

	void draw() const override
	{
		Scene::SetBackground(ColorF(0.3, 0.6, 0.9));

		FontAsset(U"TitleFont")(U"動物タワー").drawAt(400, 100);
		SimpleGUI::Button(U"Start", Vec2(350, 450));
		m_texture.drawAt(Scene::Center());
	}
	
};

//結果表示シーン
class Result :public::App::Scene {
private:

public:
	Result(const InitData& init)
		:IScene(init)
		{
	}
	void update() override {

		//フォントサイズ
		const Font font(50);
		font(U"Result").drawAt(Scene::Center(), Palette::White);

		while (System::Update()) {
			
		}
	}
};

//ゲームシーン
class Game : public App::Scene {
private:
	Texture m_texture;

public:
	Game(const InitData& init)
		:IScene(init) 
		{
	}

	void update() override {

		//登場する絵文字
		const Array<String> emojis = { U"🐘", U"🐧",U"🐑",U"🐤" };

		constexpr double scale = 0.04;

		//絵文字の形状情報とテクスチャ作成
		Array<MultiPolygon> polygons;
		Array<Texture> textures;
		for (const auto& emoji : emojis) {

			//絵文字の画像から形状情報を作成
			polygons << Emoji::CreateImage(emoji).alphaToPolygonsCentered().simplified(0.8).scale(scale);

			//絵文字の画像からテクスチャを作成
			textures << Texture(Emoji(emoji));
		}

		//物理演算用のワールド
		P2World world;

		//床
		const P2Body line = world.createStaticLine(Vec2(0, 0), Line(-12, 0, 12, 0), P2Material(1, 0.1, 1.0));

		//登場した絵文字のボディ
		Array<P2Body> bodies;

		//ボディIDと絵文字のインデックスの対応テーブル
		HashTable<P2BodyID, size_t> table;

		//2Dカメラ
		Camera2D camera(Vec2(0, -8), 20);

		//絵文字のインデックス
		size_t index = Random(polygons.size() - 1);
		
		//フォントサイズ
		const Font font(50);

		while (System::Update()) {

			ClearPrint();

			Print << Cursor::Pos(); // 現在のマウスカーソル座標を表示

			Print << U"X: " << Cursor::Pos().x; // X 座標だけを表示

			Print << U"Y: " << Cursor::Pos().y; // Y 座標だけを表示

			//物理演算ワールドの更新
			world.update();

			//2Dカメラの操作と更新
			camera.update();

			//Transformer2Dの作成
			auto t = camera.createTransformer();

			//左クリックされたら
			if (MouseL.down()) {

				// ボディを追加
				bodies << world.createPolygons(Cursor::PosF(), polygons[index], P2Material(0.1, 0.0, 1.0));

				// ボディ ID と絵文字のインデックスの対応を追加
				table.emplace(bodies.back().id(), std::exchange(index, Random(polygons.size() - 1)));

			}
			//床を描画
			line.draw(Palette::Green);
			Point pos(700, 500);
			// すべてのボディを描画
			for (const auto& body : bodies) {
				textures[table[body.id()]].scaled(scale).rotated(body.getAngle()).drawAt(body.getPos());
				if (body.getPos().y > pos.y) {
					ClearPrint();
					Print << U"Hello, Siv3D!";
					changeScene(U"Title");
				}
			}

			//現在操作できる絵文字を描画
			textures[index].scaled(scale).drawAt(Cursor::PosF(), AlphaF(0.5 + Periodic::Sine0_1(1s) * 0.5));
			//2Dカメラ操作のエフェクト表示
			camera.draw(Palette::Orange);

		}
	}
};

void Main() {

	FontAsset::Register(U"TitleFont", 60, Typeface::Heavy);

	//シーンマネージャーを作成
	App manager;

	//タイトルシーン
	manager.add<Title>(U"Title");

	//ゲームシーンの登録
	manager.add<Game>(U"Game");

	manager.add<Result>(U"Result");

	while (System::Update()) {
		if (!manager.update()) {
			break;
		}
	}

}