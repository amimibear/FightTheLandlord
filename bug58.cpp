// 鏂楀湴涓伙紙FightTheLandlord锛夋牱渚嬬▼搴?
// 鏃犺剳绛栫暐
// 鏈€鍚庢洿鏂颁簬2018-5-8
// 浣滆€咃細zhouhy
// 娓告垙淇℃伅锛歨ttp://www.botzone.org/games#FightTheLandlord

#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <cassert>
#include <cstring> // 娉ㄦ剰memset鏄痗string閲岀殑
#include <algorithm>
#include "jsoncpp/json.h" // 鍦ㄥ钩鍙颁笂锛孋++缂栬瘧鏃堕粯璁ゅ寘鍚搴?

using std::ostream;
using std::vector;
using std::sort;

using std::unique;
using std::set;
using std::string;

constexpr int PLAYER_COUNT = 3;
int what=0,io=0;//debug
int shi=0;//有没有试过放小牌

template <typename T>
ostream& operator << (ostream& o,vector<T>& v)
{
	o<<'[';
	if(v.begin()==v.end()) {o<<']'; return o;}
	for(auto it=v.begin();it!=v.rbegin();it++) o<<*it<<',';
	o<<*v.rbegin()<<']';
	return o;
}

template <typename T>
ostream& operator << (ostream& o,set<T>& v)
{
	o<<'[';
	if(v.begin()==v.end()) {o<<']'; return o;}
	auto t=v.end();
	t--;
	for(auto it=v.begin();it!=t;it++) o<<*it<<',';
	o<<*v.rbegin()<<']';
	return o;
}


struct CardCombo;

template <typename CARD_ITERATOR>
CardCombo zhudong(CARD_ITERATOR begin, CARD_ITERATOR end);
template <typename CARD_ITERATOR>
vector<vector<CardCombo>> fenpai(CARD_ITERATOR begin, CARD_ITERATOR end);


double calValue(vector<CardCombo>);
int notbest(vector<CardCombo>&);

enum class CardComboType
{
	PASS, // 杩?
	SINGLE, // 鍗曞紶
	PAIR, // 瀵瑰瓙
	STRAIGHT, // 椤哄瓙
	STRAIGHT2, // 鍙岄『
	TRIPLET, // 涓夋潯
	TRIPLET1, // 涓夊甫涓€
	TRIPLET2, // 涓夊甫浜?
	BOMB, // 鐐稿脊
	QUADRUPLE2, // 鍥涘甫浜岋紙鍙級
	QUADRUPLE4, // 鍥涘甫浜岋紙瀵癸級
	PLANE, // 椋炴満
	PLANE1, // 椋炴満甯﹀皬缈?
	PLANE2, // 椋炴満甯﹀ぇ缈?
	SSHUTTLE, // 鑸ぉ椋炴満
	SSHUTTLE2, // 鑸ぉ椋炴満甯﹀皬缈?
	SSHUTTLE4, // 鑸ぉ椋炴満甯﹀ぇ缈?
	ROCKET, // 鐏
	INVALID // 闈炴硶鐗屽瀷
};

int cardComboScores[] = {
	0, // 杩?
	1, // 鍗曞紶
	2, // 瀵瑰瓙
	6, // 椤哄瓙
	6, // 鍙岄『
	4, // 涓夋潯
	4, // 涓夊甫涓€
	4, // 涓夊甫浜?
	10, // 鐐稿脊
	8, // 鍥涘甫浜岋紙鍙級
	8, // 鍥涘甫浜岋紙瀵癸級
	8, // 椋炴満
	8, // 椋炴満甯﹀皬缈?
	8, // 椋炴満甯﹀ぇ缈?
	10, // 鑸ぉ椋炴満锛堥渶瑕佺壒鍒わ細浜岃繛涓?0鍒嗭紝澶氳繛涓?0鍒嗭級
	10, // 鑸ぉ椋炴満甯﹀皬缈?
	10, // 鑸ぉ椋炴満甯﹀ぇ缈?
	16, // 鐏
	0 // 闈炴硶鐗屽瀷
};

#ifndef _BOTZONE_ONLINE
string cardComboStrings[] = {
	"PASS",
	"SINGLE",
	"PAIR",
	"STRAIGHT",
	"STRAIGHT2",
	"TRIPLET",
	"TRIPLET1",
	"TRIPLET2",
	"BOMB",
	"QUADRUPLE2",
	"QUADRUPLE4",
	"PLANE",
	"PLANE1",
	"PLANE2",
	"SSHUTTLE",
	"SSHUTTLE2",
	"SSHUTTLE4",
	"ROCKET",
	"INVALID"
};
#endif

// 鐢?~53杩?4涓暣鏁拌〃绀哄敮涓€鐨勪竴寮犵墝
using Card = short;
constexpr Card card_joker = 52;
constexpr Card card_JOKER = 53;

// 闄や簡鐢?~53杩?4涓暣鏁拌〃绀哄敮涓€鐨勭墝锛?
// 杩欓噷杩樼敤鍙︿竴绉嶅簭鍙疯〃绀虹墝鐨勫ぇ灏忥紙涓嶇鑺辫壊锛夛紝浠ヤ究姣旇緝锛岀О浣滅瓑绾э紙Level锛?
// 瀵瑰簲鍏崇郴濡備笅锛?
// 3 4 5 6 7 8 9 10    J Q K    A    2    灏忕帇    澶х帇
// 0 1 2 3 4 5 6 7    8 9 10    11    12    13    14
using Level = short;
constexpr Level MAX_LEVEL = 15;
constexpr Level MAX_STRAIGHT_LEVEL = 11;
constexpr Level level_joker = 13;
constexpr Level level_JOKER = 14;


// 鎴戠殑鐗屾湁鍝簺
set<Card> myCards;

//鍒嗗ソ鍚庣殑鐗?
vector<CardCombo> SepCards;

// 鍦颁富琚槑绀虹殑鐗屾湁鍝簺
set<Card> landlordPublicCards;

// 澶у浠庢渶寮€濮嬪埌鐜板湪閮藉嚭杩囦粈涔?
vector<vector<Card>> whatTheyPlayed[PLAYER_COUNT];

//鍒嗙墝杩斿洖鐨勫垎鐗屾柟寮?
vector<CardCombo> sepCards;

// 澶у杩樺墿澶氬皯鐗?
short cardRemaining[PLAYER_COUNT] = { 20, 17, 17 };

// 鎴戞槸鍑犲彿鐜╁锛?-鍦颁富锛?-鍐滄皯鐢诧紝2-鍐滄皯涔欙級
int myPosition;

//============================================ 娣诲姞鐨勯儴鍒哹y鑱?=====================================================
//鐩墠鍓╀綑鐨勭墝锛宭evel涓篿dx鐨勭墝杩樺墿currRes[idx]寮?
int currRes[MAX_LEVEL + 1] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,1,1 };
//currDomain[i]琛ㄧず鐩墠鏈€澶寮犵殑Level
int currDomain[4] = { 0,14,12,12 };
//鍏朵粬浜虹洰鍓嶇殑鍓╀綑鐗?
int currotherRes[MAX_LEVEL + 1] = { 0 };
//鍏朵粬浜虹洰鍓嶇殑鏈€澶х墝
int currotherDomain[4] = { 0 };

//鏁屾柟缂哄皯鐨勭墝鍨?
bool enemyLack[18] = {};


/**
 * 灏咰ard鍙樻垚Level
 */
constexpr Level card2level(Card card)
{
	return card / 4 + card / 53;
}
set<Card> remain_card;
void insert_all_card()
{
	for (int i = 0; i < 53; ++i)
		remain_card.insert(Card(i));
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < whatTheyPlayed[i].size(); ++j)
			for (int k = 0; k < whatTheyPlayed[i][j].size(); ++k)
			{
				remain_card.erase(whatTheyPlayed[i][j][k]);
			}
	}
	for (auto i = myCards.begin(); i != myCards.end(); ++i)
	{
		remain_card.erase(*i);
	}
	return;
}

/*bool judge_best()
 {
 if (CardCombo(bestcard.begin(), bestcard.end()).findFirstValid(remain_card.begin(), remain_card.end()).comboType == CardComboType::PASS)
 return true;
 else
 return false;
 }*/
// 鐗岀殑缁勫悎锛岀敤浜庤绠楃墝鍨?
struct CardCombo
{
	// 琛ㄧず鍚岀瓑绾х殑鐗屾湁澶氬皯寮?
	// 浼氭寜涓暟浠庡ぇ鍒板皬銆佺瓑绾т粠澶у埌灏忔帓搴?
	struct CardPack
	{
		Level level;
		short count;
		
		bool operator< (const CardPack& b) const
		{
			if (count == b.count)
				return level > b.level;
			return count > b.count;
		}
	};
	vector<Card> cards; // 鍘熷鐨勭墝锛屾湭鎺掑簭
	vector<CardPack> packs; // 鎸夋暟鐩拰澶у皬鎺掑簭鐨勭墝绉?
	CardComboType comboType; // 绠楀嚭鐨勭墝鍨?
	int comboTypeInt;//鐗屽瀷瀵瑰簲鐨刬ndex
	Level comboLevel = 0; // 绠楀嚭鐨勫ぇ灏忓簭
	
	/**
	 * 妫€鏌ヤ釜鏁版渶澶氱殑CardPack閫掑噺浜嗗嚑涓?
	 */
	int findMaxSeq() const
	{
		for (unsigned c = 1; c < packs.size(); c++)
			if (packs[c].count != packs[0].count ||
				packs[c].level != packs[c - 1].level - 1)
				return c;
		return (int)packs.size();
	}
	
	/**
	 * 杩欎釜鐗屽瀷鏈€鍚庣畻鎬诲垎鐨勬椂鍊欑殑鏉冮噸鐔
	 */
	int getWeight() const
	{
		if (comboType == CardComboType::SSHUTTLE ||
			comboType == CardComboType::SSHUTTLE2 ||
			comboType == CardComboType::SSHUTTLE4)
			return cardComboScores[(int)comboType] + (findMaxSeq() > 2) * 10;
		return cardComboScores[(int)comboType];
	}
	
	// 鍒涘缓涓€涓┖鐗岀粍
	CardCombo() : comboType(CardComboType::PASS), comboTypeInt(0) {}
	
	/**
	 * 閫氳繃Card锛堝嵆short锛夌被鍨嬬殑杩唬鍣ㄥ垱寤轰竴涓墝鍨?
	 * 骞惰绠楀嚭鐗屽瀷鍜屽ぇ灏忓簭绛?
	 * 鍋囪杈撳叆娌℃湁閲嶅鏁板瓧锛堝嵆閲嶅鐨凜ard锛?
	 */
	template <typename CARD_ITERATOR>
	CardCombo(CARD_ITERATOR begin, CARD_ITERATOR end)
	{
		// 鐗瑰垽锛氱┖
		if (begin == end)
		{
			comboType = CardComboType::PASS;
			comboTypeInt = 0;
			return;
		}
		
		// 姣忕鐗屾湁澶氬皯涓?
		short counts[MAX_LEVEL + 1] = {};
		
		// 鍚岀鐗岀殑寮犳暟锛堟湁澶氬皯涓崟寮犮€佸瀛愩€佷笁鏉°€佸洓鏉★級
		short countOfCount[5] = {};
		
		cards = vector<Card>(begin, end);
		for (Card c : cards)
			counts[card2level(c)]++;
		for (Level l = 0; l <= MAX_LEVEL; l++)
			if (counts[l])
			{
				packs.push_back(CardPack{ l, counts[l] });
				countOfCount[counts[l]]++;
			}
		sort(packs.begin(), packs.end());
		
		// 鐢ㄦ渶澶氱殑閭ｇ鐗屾€绘槸鍙互姣旇緝澶у皬鐨?
		comboLevel = packs[0].level;
		
		// 璁＄畻鐗屽瀷
		// 鎸夌収 鍚岀鐗岀殑寮犳暟 鏈夊嚑绉?杩涜鍒嗙被
		vector<int> kindOfCountOfCount;
		for (int i = 0; i <= 4; i++)
			if (countOfCount[i])
				kindOfCountOfCount.push_back(i);
		sort(kindOfCountOfCount.begin(), kindOfCountOfCount.end());
		
		int curr, lesser;
		
		switch (kindOfCountOfCount.size())
		{
			case 1: // 鍙湁涓€绫荤墝
				curr = countOfCount[kindOfCountOfCount[0]];
				switch (kindOfCountOfCount[0])
			{
				case 1:
					// 鍙湁鑻ュ共鍗曞紶
					if (curr == 1)
					{
						comboType = CardComboType::SINGLE;
						comboTypeInt = 1;
						return;
					}
					if (curr == 2 && packs[1].level == level_joker)
					{
						comboType = CardComboType::ROCKET;
						comboTypeInt = 17;
						return;
					}
					if (curr >= 5 && findMaxSeq() == curr &&
						packs.begin()->level <= MAX_STRAIGHT_LEVEL)
					{
						comboType = CardComboType::STRAIGHT;
						comboTypeInt = 3;
						return;
					}
					break;
				case 2:
					// 鍙湁鑻ュ共瀵瑰瓙
					if (curr == 1)
					{
						comboType = CardComboType::PAIR;
						comboTypeInt = 2;
						return;
					}
					if (curr >= 3 && findMaxSeq() == curr &&
						packs.begin()->level <= MAX_STRAIGHT_LEVEL)
					{
						comboType = CardComboType::STRAIGHT2;
						comboTypeInt = 4;
						return;
					}
					break;
				case 3:
					// 鍙湁鑻ュ共涓夋潯
					if (curr == 1)
					{
						comboType = CardComboType::TRIPLET;
						comboTypeInt = 5;
						return;
					}
					if (findMaxSeq() == curr &&
						packs.begin()->level <= MAX_STRAIGHT_LEVEL)
					{
						comboType = CardComboType::PLANE;
						comboTypeInt = 11;
						return;
					}
					break;
				case 4:
					// 鍙湁鑻ュ共鍥涙潯
					if (curr == 1)
					{
						comboType = CardComboType::BOMB;
						comboTypeInt = 8;
						return;
					}
					if (findMaxSeq() == curr &&
						packs.begin()->level <= MAX_STRAIGHT_LEVEL)
					{
						comboType = CardComboType::SSHUTTLE;
						comboTypeInt = 14;
						return;
					}
			}
				break;
			case 2: // 鏈変袱绫荤墝
				curr = countOfCount[kindOfCountOfCount[1]];
				lesser = countOfCount[kindOfCountOfCount[0]];
				if (kindOfCountOfCount[1] == 3)
				{
					// 涓夋潯甯︼紵
					if (kindOfCountOfCount[0] == 1)
					{
						// 涓夊甫涓€
						if (curr == 1 && lesser == 1)
						{
							comboType = CardComboType::TRIPLET1;
							comboTypeInt = 6;
							return;
						}
						if (findMaxSeq() == curr && lesser == curr &&
							packs.begin()->level <= MAX_STRAIGHT_LEVEL)
						{
							comboType = CardComboType::PLANE1;
							comboTypeInt = 12;
							return;
						}
					}
					if (kindOfCountOfCount[0] == 2)
					{
						// 涓夊甫浜?
						if (curr == 1 && lesser == 1)
						{
							comboType = CardComboType::TRIPLET2;
							comboTypeInt = 7;
							return;
						}
						if (findMaxSeq() == curr && lesser == curr &&
							packs.begin()->level <= MAX_STRAIGHT_LEVEL)
						{
							comboType = CardComboType::PLANE2;
							comboTypeInt = 13;
							return;
						}
					}
				}
				if (kindOfCountOfCount[1] == 4)
				{
					// 鍥涙潯甯︼紵
					if (kindOfCountOfCount[0] == 1)
					{
						// 鍥涙潯甯︿袱鍙?* n
						if (curr == 1 && lesser == 2)
						{
							comboType = CardComboType::QUADRUPLE2;
							comboTypeInt = 9;
							return;
						}
						if (findMaxSeq() == curr && lesser == curr * 2 &&
							packs.begin()->level <= MAX_STRAIGHT_LEVEL)
						{
							comboType = CardComboType::SSHUTTLE2;
							comboTypeInt = 15;
							return;
						}
					}
					if (kindOfCountOfCount[0] == 2)
					{
						// 鍥涙潯甯︿袱瀵?* n
						if (curr == 1 && lesser == 2)
						{
							comboType = CardComboType::QUADRUPLE4;
							comboTypeInt = 10;
							return;
						}
						if (findMaxSeq() == curr && lesser == curr * 2 &&
							packs.begin()->level <= MAX_STRAIGHT_LEVEL)
						{
							comboType = CardComboType::SSHUTTLE4;
							comboTypeInt = 16;
							return;
						}
					}
				}
		}
		
		comboType = CardComboType::INVALID;
		comboTypeInt = 18;
	}
	
	/**
	 * 鍒ゆ柇鎸囧畾鐗岀粍鑳藉惁澶ц繃褰撳墠鐗岀粍锛堣繖涓嚱鏁颁笉鑰冭檻杩囩墝鐨勬儏鍐碉紒锛?
	 */
	bool canBeBeatenBy(const CardCombo& b) const
	{
		if (comboType == CardComboType::INVALID || b.comboType == CardComboType::INVALID)
			return false;
		if (b.comboType == CardComboType::ROCKET)
			return true;
		if (b.comboType == CardComboType::BOMB)
			switch (comboType)
		{
			case CardComboType::ROCKET:
				return false;
			case CardComboType::BOMB:
				return b.comboLevel > comboLevel;
			default:
				return true;
		}
		return b.comboType == comboType && b.cards.size() == cards.size() && b.comboLevel > comboLevel;
	}
	
	/**
	 * 浠庢寚瀹氭墜鐗屼腑瀵绘壘绗竴涓兘澶ц繃褰撳墠鐗岀粍鐨勭墝缁?
	 * 濡傛灉闅忎究鍑虹殑璇濆彧鍑虹涓€寮?
	 * 濡傛灉涓嶅瓨鍦ㄥ垯杩斿洖涓€涓狿ASS鐨勭墝缁?
	 */
	
	template <typename CARD_ITERATOR>
	CardCombo findFirstValid00(CARD_ITERATOR begin, CARD_ITERATOR end) const
	{
		if (comboType == CardComboType::PASS) // 如果不需要大过谁，只需要随便出
		{
			CARD_ITERATOR second = begin;
			second++;
			return CardCombo(begin, second); // 那么就出第一张牌……
		}
		
		// 然后先看一下是不是火箭，是的话就过
		if (comboType == CardComboType::ROCKET)
			return CardCombo();
		
		// 现在打算从手牌中凑出同牌型的牌
		auto deck = vector<Card>(begin, end); // 手牌
		short counts[MAX_LEVEL + 1] = {};
		
		unsigned short kindCount = 0;
		
		// 先数一下手牌里每种牌有多少个
		for (Card c : deck)
			counts[card2level(c)]++;
		
		// 手牌如果不够用，直接不用凑了，看看能不能炸吧
		if (deck.size() < cards.size())
			goto failure;
		
		// 再数一下手牌里有多少种牌
		for (short c : counts)
			if (c)
				kindCount++;
		
		// 否则不断增大当前牌组的主牌，看看能不能找到匹配的牌组
		{
			// 开始增大主牌
			int mainPackCount = findMaxSeq();
			bool isSequential =
			comboType == CardComboType::STRAIGHT ||
			comboType == CardComboType::STRAIGHT2 ||
			comboType == CardComboType::PLANE ||
			comboType == CardComboType::PLANE1 ||
			comboType == CardComboType::PLANE2 ||
			comboType == CardComboType::SSHUTTLE ||
			comboType == CardComboType::SSHUTTLE2 ||
			comboType == CardComboType::SSHUTTLE4;
			for (Level i = 1; ; i++) // 增大多少
			{
				for (int j = 0; j < mainPackCount; j++)
				{
					int level = packs[j].level + i;
					
					// 各种连续牌型的主牌不能到2，非连续牌型的主牌不能到小王，单张的主牌不能超过大王
					if ((comboType == CardComboType::SINGLE && level > MAX_LEVEL) ||
						(isSequential && level > MAX_STRAIGHT_LEVEL) ||
						(comboType != CardComboType::SINGLE && !isSequential && level >= level_joker))
						goto failure;
					
					// 如果手牌中这种牌不够，就不用继续增了
					if (counts[level] < packs[j].count)
						goto next;
				}
				
				{
					// 找到了合适的主牌，那么从牌呢？
					// 如果手牌的种类数不够，那从牌的种类数就不够，也不行
					if (kindCount < packs.size())
						continue;
					
					// 好终于可以了
					// 计算每种牌的要求数目吧
					short requiredCounts[MAX_LEVEL + 1] = {};
					for (int j = 0; j < mainPackCount; j++)
						requiredCounts[packs[j].level + i] = packs[j].count;
					for (unsigned j = mainPackCount; j < packs.size(); j++)
					{
						Level k;
						for (k = 0; k <= MAX_LEVEL; k++)
						{
							if (requiredCounts[k] || counts[k] < packs[j].count)
								continue;
							requiredCounts[k] = packs[j].count;
							break;
						}
						if (k == MAX_LEVEL + 1) // 如果是都不符合要求……就不行了
							goto next;
					}
					
					
					// 开始产生解
					vector<Card> solve;
					for (Card c : deck)
					{
						Level level = card2level(c);
						if (requiredCounts[level])
						{
							solve.push_back(c);
							requiredCounts[level]--;
						}
					}
					return CardCombo(solve.begin(), solve.end());
				}
				
			next:
				; // 再增大
			}
		}
		
	failure:
		// 实在找不到啊
		// 最后看一下能不能炸吧
		
		for (Level i = 0; i < level_joker; i++)
			if (counts[i] == 4 && (comboType != CardComboType::BOMB || i > packs[0].level)) // 如果对方是炸弹，能炸的过才行
			{
				// 还真可以啊……
				Card bomb[] = { Card(i * 4), Card(i * 4 + 1), Card(i * 4 + 2), Card(i * 4 + 3) };
				return CardCombo(bomb, bomb + 4);
			}
		
		// 有没有火箭？
		if (counts[level_joker] + counts[level_JOKER] == 2)
		{
			Card rocket[] = { card_joker, card_JOKER };
			return CardCombo(rocket, rocket + 2);
		}
		return CardCombo();
	}
	
	//=========================================淇敼CardCombo鍑芥暟byqianqian======================================================
	template <typename CARD_ITERATOR>
	CardCombo findFirstValid(CARD_ITERATOR begin, CARD_ITERATOR end) const
	{
		if (comboType == CardComboType::PASS)
		{
			return zhudong(begin, end);
		}
		CardCombo bomb0[13];
		CardCombo rocket0;
		CardCombo my_card(begin, end);//判断能不能一手出完；
		set<Card> temp_card(begin, end);
		set<Card>::iterator it;
		vector<Card>::iterator it0;
		double nowMax = -1000;
		vector<vector<Card>> solve;
		if (my_card.comboType != CardComboType::INVALID&&this->canBeBeatenBy(my_card))
		{
			return my_card;//如果一手出完且能接上，就直接出；
		}
		//对方是火箭，直接过；
		if (comboType == CardComboType::ROCKET)
			return CardCombo();
		
		// 艙梅鈥樷亜楼脷脌鈥灺モ€澛犆封増鈭喢封€撀モ€欌墺藛脮篓鈮堚垎鈥撁暵灯掆増鈭?
		auto deck = vector<Card>(begin, end); // 聽梅鈮堚垎
		short counts[MAX_LEVEL + 1] = {};
		
		unsigned short kindCount = 0;
		
		for (Card c : deck)
			counts[card2level(c)]++;
		
		/*if (deck.size() < cards.size())
		 goto failure;*/
		
		for (short c : counts)
			if (c)
				kindCount++;
		
		//分离炸弹和火箭
		
		if (counts[level_joker] + counts[level_JOKER] == 2)
		{
			Card rocket[] = { card_joker, card_JOKER };
			rocket0 = CardCombo(rocket, rocket + 2);
			counts[13] = counts[14] = 0;
			temp_card.erase(card_joker); temp_card.erase(card_JOKER);
		}
		for (Level i = 0; i < level_joker; i++)
		{
			if (counts[i] == 4)
			{
				Card bomb[] = { Card(i * 4), Card(i * 4 + 1), Card(i * 4 + 2), Card(i * 4 + 3) };
				temp_card.erase(i * 4); temp_card.erase(i * 4 + 1); temp_card.erase(i * 4 + 2); temp_card.erase(i * 4 + 3);
				bomb0[i] = CardCombo(bomb, bomb + 4);
				counts[i] = 0;
			}
		}
		
		CardCombo temp_card0(temp_card.begin(), temp_card.end());
		if (temp_card0.comboType != CardComboType::INVALID)
		{
			if (rocket0.comboType != CardComboType::PASS)
			{
				return rocket0;
			}
			for (int i = 0; i < level_joker; ++i)
			{
				if (bomb0[i].comboType != CardComboType::PASS&&this->canBeBeatenBy(bomb0[i]))
				{
					return bomb0[i];
				}
			}
		}
		//std::cout << "炸他庞麦郎！";
		
		
		
		
		{
			int mainPackCount = findMaxSeq();
			bool isSequential =
			comboType == CardComboType::STRAIGHT ||
			comboType == CardComboType::STRAIGHT2 ||
			comboType == CardComboType::PLANE ||
			comboType == CardComboType::PLANE1 ||
			comboType == CardComboType::PLANE2 ||
			comboType == CardComboType::SSHUTTLE ||
			comboType == CardComboType::SSHUTTLE2 ||
			comboType == CardComboType::SSHUTTLE4;//酶鈥ζ掆€光墹陋鈥濃垰鈥櫬得樜┾€澛灯掆増鈭喡Ｂ?
			for (Level i = 1;; i++) // 鈥標喡ッ涒垈鈥♀€ε?
			{
				for (int j = 0; j < mainPackCount; j++)
				{
					int level = packs[j].level + i;
					
					// 鈭徦溍访仿÷ㄢ€撀増鈭嗏€撁暵灯捗匪溾増鈭嗏墹陋茠鈥孤滴?拢篓鈭懧÷ㄢ€撀増鈭嗏€撁暵灯捗匪溾増鈭嗏墹陋茠鈥孤滴┾€撀懊暷甭Ｂ碘€⑩€欌増碌茠梅藴鈮堚垎鈮ぢ掆€光墺篓蟺藵楼脹脮谋
					if ((comboType == CardComboType::SINGLE && level > MAX_LEVEL) ||
						(isSequential && level > MAX_STRAIGHT_LEVEL) ||
						(comboType != CardComboType::SINGLE && !isSequential && level >= level_joker))
						goto failure;
					
					// 禄脕蟺藲聽梅鈮堚垎梅鈥撯€欌€毭访封増鈭嗏墹陋蟺陋拢篓忙脮鈮ぢ€濃垰潞脙鈥撀€標喡∶€
					if (counts[level] < packs[j].count)
						goto next;
				}
				
				{
					if (kindCount < packs.size())
						continue;
					
					short requiredCounts[MAX_LEVEL + 1] = {};
					for (int j = 0; j < mainPackCount; j++)
						requiredCounts[packs[j].level + i] = packs[j].count;
					for (unsigned j = mainPackCount; j < packs.size(); j++)
					{
						Level k;
						for (k = 0; k <= MAX_LEVEL; k++)
						{
							if (requiredCounts[k] || counts[k] < packs[j].count)
								continue;
							requiredCounts[k] = packs[j].count;
							break;
						}
						if (k == MAX_LEVEL + 1)
							goto next;
					}
					
					
					// 酶鈩⒙犅衡墹藱鈥λ櫸┾€?
					vector<Card> tmp;
					for (Card c : deck)
					{
						Level level = card2level(c);
						if (requiredCounts[level])
						{
							tmp.push_back(c);
							requiredCounts[level]--;
						}
					}
					solve.push_back(tmp);
				}
			next:
				;
			}
		}
		
		//std::cout << "this is next" << std::endl; // 鈥樑糕€標喡ッ?
	failure:
		//std::cout << num << std::endl;
		double pass=-1000;
		vector<vector<CardCombo>> all = fenpai(begin, end);
		for (auto i = all.begin(); i != all.end(); i++)//i代表每种分牌方式
		{
			double tmppass=calValue(*i);
			if(tmppass>pass) pass=tmppass;
			for (auto j = i->begin(); j != i->end(); j++)//j表示里面的cardcombo
				if(canBeBeatenBy(*j))
					solve.push_back(j->cards);
		}
		CardCombo best;
		CardCombo da;
		double daa=-1000;
		int you=0;//有没有不是炸的解
		for (int s0 = 0; s0 < solve.size(); s0++)
		{
			set<Card> temp_cards(begin, end);
			CARD_ITERATOR it;
			for (it0 = solve[s0].begin(); it0 != solve[s0].end(); ++it0)
			{
				temp_cards.erase(*it0);
			}
			CardCombo temp1(solve[s0].begin(), solve[s0].end());//自己给出的一个解；
			//temp1.debugPrint();
			CardCombo temp2(temp_cards.begin(), temp_cards.end());//自己剩余手牌；
			//std::cout << solve[s0][0] << "   ";
			/************************/
			//特判自己的牌是全场最大，且只有两手牌；
			if ((temp1.findFirstValid00(remain_card.begin(), remain_card.end()).comboType == CardComboType::PASS) && (temp2.comboType != CardComboType::INVALID))
			{
				//std::cout<<s0<<' ';
				return temp1;
			}
			set<Card>::iterator it1;
			vector<vector<CardCombo> > result0 = fenpai(temp_cards.begin(), temp_cards.end());
			double tmpmax = -1000;
			for (auto i = result0.begin(); i != result0.end(); ++i)
			{
				double tmp=calValue(*i);
				//if(temp1.isbest()) tmp+=0.5;//tiao??
				
				if(tmp>tmpmax) tmpmax = tmp;
				if(temp1.isbest()&&(notbest(*i)<=1)) return temp1;
			}
			if(temp1.isbest()&&tmpmax>daa) {da=temp1; daa=tmpmax;}//tiao
			if(temp1.comboType == CardComboType::BOMB||temp1.comboType == CardComboType::ROCKET) continue;
			you=1;
			//std::cout<<nowMax<<' ';
			if (tmpmax > nowMax)
			{
				//std::cout<<'!';
				nowMax = tmpmax;
				best = temp1;
				//std::cout<<s0<<' ';
			}
		}
		if (myPosition == 1 && whatTheyPlayed[0].rbegin()->size() == 0 && whatTheyPlayed[2].rbegin()->size() != 0
			&& cardRemaining[2]<=2) return CardCombo();
		if(myPosition==1 && cardRemaining[2]==1 && !shi && da.comboType != CardComboType::PASS) return da;
		if ((myPosition == 2 && whatTheyPlayed[1].rbegin()->size() != 0)//不打队友
			|| (myPosition == 1 && whatTheyPlayed[0].rbegin()->size() == 0 && whatTheyPlayed[2].rbegin()->size() != 0))
		{
			if (
				comboType == CardComboType::QUADRUPLE2 ||
				comboType == CardComboType::QUADRUPLE4 ||
				comboType == CardComboType::SSHUTTLE ||
				comboType == CardComboType::SSHUTTLE2 ||
				comboType == CardComboType::SSHUTTLE4 ||
				comboType == CardComboType::BOMB
				)
				return CardCombo();
			if ((comboType == CardComboType::SINGLE) || (comboType == CardComboType::STRAIGHT))
			{
				int temp1[15] = { 0 }; int s = 0;
				for (int t = 14; t >= 0; --t)
				{
					if (currotherRes[t] >= 1)
					{
						temp1[s] = t; s++;
					}
				}
				if (packs[0].level >= temp1[1] || packs[0].level >= 11)
				{
					return CardCombo();
				}
			}
			if (comboType == CardComboType::STRAIGHT2 || comboType == CardComboType::PAIR)
			{
				int temp2[13] = { 0 }; int s = 0;
				for (int t = 12; t >= 0; --t)
				{
					if (currotherRes[t] >= 2)
					{
						temp2[s] = t; ++s;
					}
				}
				if (packs[0].level >= temp2[2] || packs[0].level >= 9)
				{
					return CardCombo();
				}
			}
			if (comboType == CardComboType::TRIPLET || comboType == CardComboType::TRIPLET1 || comboType == CardComboType::TRIPLET2 || comboType == CardComboType::PLANE || comboType == CardComboType::PLANE1 || comboType == CardComboType::PLANE2)
			{
				int temp3[14] = { 0 }; int s = 0;
				for (int t = 12; t >= 0; --t)
				{
					if (currotherRes[t] >= 3)
					{
						temp3[s] = t; ++s;
					}
				}
				if (packs[0].level >= temp3[2])
				{
					return CardCombo();
				}
			}
		}
		//门板挡地主
		if (comboType == CardComboType::SINGLE && myPosition == 2 && whatTheyPlayed[1][whatTheyPlayed[1].size() - 1].size() != 0
			&&whatTheyPlayed[1][whatTheyPlayed[1].size() - 1][0] < 28)
		{
			double now_max = -1000;
			CardCombo result;
			for (int k = 0; k < solve.size(); ++k)
			{
				if (solve[k][0] >= 28 && solve[k][0] <= 43)
				{
					set<Card> tempMycards = myCards;
					tempMycards.erase(solve[k][0]);
					vector<vector<CardCombo>> res = fenpai(tempMycards.begin(),tempMycards.end());
					double thismax = -1000;
					for(auto i = res.begin();i != res.end();++i)
					{
						double now_val=calValue(*i);
						if(now_val>thismax)
						{
							thismax = now_val;
						}
					}
					if(thismax > now_max)
					{
						now_max = thismax;
						result = CardCombo(solve[k].begin(),solve[k].end());
					}
					
					//return CardCombo(solve[k].begin(), solve[k].end());
				}
			}
			if(result.comboType != CardComboType::PASS)
				return result;
		}
		
		if(you)
		{
			nowMax+=0.7;//tiao3
			if(best.isbest()) nowMax+=0.5;//tiao3
			if(myPosition==0 && comboType==CardComboType::SINGLE && (cardRemaining[1]==1||cardRemaining[2]==1)) return best;
			//if(myPosition==0 && comboType==CardComboType::PAIR && (cardRemaining[1]==2||cardRemaining[2]==2)) return best;
			if(myPosition==0 && nowMax<pass) return CardCombo();
			return best;
		}
		if(myPosition==0&&pass<0) return CardCombo();//tiao2
		if ((myPosition == 2 && whatTheyPlayed[1].rbegin()->size() != 0)//不炸队友
			|| (myPosition == 1 && whatTheyPlayed[0].rbegin()->size() == 0 && whatTheyPlayed[2].rbegin()->size() != 0))
		{
			return CardCombo();
		}
		for (int i = 0; i < 13; ++i)
		{
			if (bomb0[i].comboType != CardComboType::PASS&&canBeBeatenBy(bomb0[i]))
			{
				return bomb0[i];
			}
		}
		if (rocket0.comboType != CardComboType::PASS)
		{
			return rocket0;
		}
		
		return CardCombo();
	}
	
	bool isbest()
	{
		return findFirstValid00(remain_card.begin(), remain_card.end()).comboType == CardComboType::PASS;
	}
	
	void debugPrint()
	{
#ifndef _BOTZONE_ONLINE
		std::cout << "【" << cardComboStrings[(int)comboType] <<
		"共" << cards.size() << "张，大小序" << comboLevel << "】";
#endif
	}
	
	friend ostream& operator << (ostream& o,CardCombo& v)
	{
		o<<'[';
		if(v.cards.begin()==v.cards.end()) {o<<']'; return o;}
		for(auto it=v.cards.begin();it!=v.cards.end()-1;it++) o<<*it<<',';
		o<<*v.cards.rbegin()<<']';
		return o;
	}
};

// 褰撳墠瑕佸嚭鐨勭墝闇€瑕佸ぇ杩囪皝
CardCombo lastValidCombo;

void updateDomain()
{
	bool flag1 = false, flag2 = false, flag3 = false;
	for (int i = 14; i >= 0; --i)
	{
		if (currRes[i] && !flag1)
		{
			currDomain[1] = i;
			flag1 = true;
		}
		if (currRes[i] >= 2 && !flag2)
		{
			currDomain[2] = i;
			flag2 = true;
		}
		if (currRes[i] >= 3 && !flag3)
		{
			currDomain[3] = i;
			flag3 = true;
		}
		if (flag1&&flag2&&flag3)
			break;
	}
	for (int i = 0; i < MAX_LEVEL + 1; ++i) currotherRes[i] = currRes[i];
	for (auto it = myCards.begin(); it != myCards.end(); ++it)
	{
		--currotherRes[card2level(*it)];
	}
	flag1 = false; flag2 = false; flag3 = false;
	for (int i = 14; i >= 0; --i)
	{
		if (currotherRes[i] && !flag1)
		{
			currDomain[1] = i;
			flag1 = true;
		}
		if (currotherRes[i] >= 2 && !flag2)
		{
			currotherDomain[2] = i;
			flag2 = true;
		}
		if (currotherRes[i] >= 3 && !flag3)
		{
			currotherDomain[3] = i;
			flag3 = true;
		}
		if (flag1&&flag2&&flag3)
			break;
	}
}

//==================================================鍒嗙墝bzytty================================================
int a[4] = { 0,5,3,2 };
void zuhe(vector<vector<CardCombo>>& resres, vector<CardCombo>& res, vector<CardCombo>& three, set<Card>& s,vector<CardCombo>& four)
{
	int counts[MAX_LEVEL + 1] = {};
	for (auto i = s.begin(); i != s.end(); i++) counts[card2level(*i)]++;
	
	//for(auto i=res.begin();i!=res.end();i++) i->debugPrint(); std::cout<<std::endl;
	auto tmpres=res;
	for(auto thr:three) tmpres.push_back(thr);
	for(auto fur:four) tmpres.push_back(fur);
	for(int i=0;i<=12;i++) if(counts[i])
	{
		vector<Card> tmp;
		for(auto it=s.lower_bound(4*i);it!=s.upper_bound(4*i+3);it++) tmp.push_back(*it);
		tmpres.push_back(CardCombo(tmp.begin(),tmp.end()));
	}
	vector<Card> tmp;
	if(s.count(52)) tmp.push_back(52);
	if(s.count(53)) tmp.push_back(53);
	if(tmp.begin()!=tmp.end()) tmpres.push_back(CardCombo(tmp.begin(),tmp.end()));
	resres.push_back(tmpres);
	//for(auto i=tmpres.begin();i!=tmpres.end();i++) i->debugPrint(); std::cout<<std::endl;
	
	for (int i = 1; i <= 2; i++)
	{
		for(auto j=three.begin();j!=three.end();j++)
		{
			int n = 0;
			int d[15];
			for (int k = 0; k <= 14; k++) if (counts[k] == i)
			{
				int o=0;
				for(int l=0;l<j->findMaxSeq();l++) if(k==j->packs[l].level) o=1;
				if(o) continue;
				d[n++] = k;
				if (n == j->findMaxSeq()) break;
			}
			if (n == j->findMaxSeq())
			{
				vector<CardCombo> three_ = three;
				vector<CardCombo> four_ = four;
				vector<CardCombo> res_ = res;
				set<Card> s_ = s;
				three_.erase(j-three.begin()+three_.begin());
				vector<int> tmp(j->cards.begin(), j->cards.end());
				for (int k = 0; k<j->findMaxSeq(); k++)
				{
					auto l = s_.lower_bound(4 * d[k] - (d[k]/14)*3);
					auto r = s_.upper_bound(4 * d[k] + 3 -(d[k]/13)*3 - (d[k]/14)*3);
					for (auto it = l; it != r; it++) tmp.push_back(*it);
					s_.erase(l, r);
				}
				res_.push_back(CardCombo(tmp.begin(), tmp.end()));
				zuhe(resres, res_, three_, s_,four_);
			}
		}
		for(auto j=four.begin();j!=four.end();j++)
		{
			int n = 0;
			int d[15];
			for (int k = 0; k <= 14; k++) if (counts[k] == i) { d[n++] = k; if (n == 2) break;}
			if (n == 2)
			{
				vector<CardCombo> three_ = three;
				vector<CardCombo> four_ = four;
				vector<CardCombo> res_ = res;
				set<Card> s_ = s;
				four_.erase(j-four.begin()+four_.begin());
				vector<int> tmp(j->cards.begin(), j->cards.end());
				for (int k = 0; k < 2; k++)
				{
					auto l = s_.lower_bound(4 * d[k] - (d[k]/14)*3);
					auto r = s_.upper_bound(4 * d[k] + 3 -(d[k]/13)*3 - (d[k]/14)*3);
					for (auto it = l; it != r; it++) tmp.push_back(*it);
					s_.erase(l, r);
				}
				res_.push_back(CardCombo(tmp.begin(), tmp.end()));
				zuhe(resres, res_, three_, s_,four_);
			}
		}
	}
}

void fen(vector<vector<CardCombo>>& resres, vector<CardCombo>& res, vector<CardCombo>& three, set<Card>& s, int* bb)
{
	int counts[MAX_LEVEL + 1] = {};
	for (auto i = s.begin(); i != s.end(); i++) counts[card2level(*i)]++;
	vector<CardCombo> tmpres = res;
	vector<CardCombo> tmpthree = three;
	vector<CardCombo> four;
	set<Card> tmps = s;
	for (int i = 0; i <= 14; i++) if (counts[i] == 3)
	{
		vector<Card> tmp;
		auto l = tmps.lower_bound(4 * i);
		auto r = tmps.upper_bound(4 * i + 3);
		for (auto it = l; it != r; it++) tmp.push_back(*it);
		tmps.erase(l, r);
		tmpthree.push_back(CardCombo(tmp.begin(), tmp.end()));
	}
	for (int i = 0; i <= 14; i++) if (counts[i] == 4)
	{
		vector<Card> tmp;
		auto l = tmps.lower_bound(4 * i);
		auto r = tmps.upper_bound(4 * i + 3);
		for (auto it = l; it != r; it++) tmp.push_back(*it);
		tmps.erase(l, r);
		four.push_back(CardCombo(tmp.begin(), tmp.end()));
	}
	zuhe(resres, tmpres, tmpthree, tmps,four);
	
	
	int b[4];//b琛ㄧず鏈€澶у彲鑳芥湁鍑犻『
	for (int i = 0; i <= 3; i++) b[i] = bb[i];
	
	for (int i = 1; i <= 3; i++)
		for (int j = a[i]; j <= b[i]; j++)
		{
			int oo = 1;
			for (int k = 0; k <= 12 - j; k++)
			{
				int o = 1;
				for (int l = k; l<k + j; l++) if (counts[l]<i) o = 0;
				if (o)
				{
					oo = 0;
					vector<Card> tmp;
					vector<CardCombo> res_ = res;
					vector<CardCombo> three_ = three;
					set<Card> s_ = s;
					for (int l = k; l<k + j; l++)
					{
						//auto it=s_.lower_bound(4*l);
						for (int m = 0; m<i; m++)
						{
							auto it = s_.lower_bound(4 * l);
							tmp.push_back(*it);
							s_.erase(it);
						}
					}
					if (i == 3) three_.push_back(CardCombo(tmp.begin(), tmp.end()));
					else res_.push_back(CardCombo(tmp.begin(), tmp.end()));
					fen(resres, res_, three_, s_, b);
				}
			}
			if (oo) { b[i] = j - 1; break; }
		}
}

template <typename CARD_ITERATOR>
vector<vector<CardCombo>> fenpai(CARD_ITERATOR begin, CARD_ITERATOR end)
{
	vector<vector<CardCombo>> resres;
	vector<CardCombo> res;
	set<Card> s;
	s.insert(begin, end);
	int b[4] = { 0,12,10,6 };
	vector<CardCombo> three;
	fen(resres, res, three, s, b);
	return resres;
}
//===============================================================涓诲姩鍑虹墝浼板€糱y鑱?=======================================================

int notbest(vector<CardCombo>& v)
{
	int n=0;
	for(auto i=v.begin();i!=v.end();i++) if(!i->isbest()) n++;
	return n;
}

double getvalue(CardCombo c)
{
	double val = 0;
	//if(c.comboTypeInt!=0&&c.isbest()) val+=0.5;
	switch (c.comboTypeInt)
	{
		case 1:
			if (c.comboLevel <= 9)//Q及以下
				val -= 1;
			else if (c.comboLevel<=11) ;//KA
			else if (c.comboLevel<=12)//2
				val+= 0.3;
			else if (c.comboLevel<=13)//小王
				val += 0.5;
			else if (c.comboLevel == 14)//大王
				val += 1;
			break;
		case 2:
			if (c.comboLevel <= 9)//Q及以下
				val -= 1;
			else if (c.comboLevel<=10) ;
			else if (c.comboLevel<=11) val += 0.4;//tiao4
			else if (c.comboLevel<=12)//2
				val += 1;
			break;
		case 3://顺子
			//if(c.comboTypeInt!=0)
			val += -1-c.findMaxSeq()*0.1+c.comboLevel*0.1;//tiao5
			//val += -0.5;
			break;
		case 4://双顺
			val += -1.6+c.comboLevel*0.1;
			//val-=0.5;
			break;
		case 5://三条
			//val -= 0.5;
			
			if (c.comboLevel <= 9)
				val -= 0.5;
			else if (c.comboLevel<=10) ;
			else if (c.comboLevel<=11)
				val+= 0.5;
			else if (c.comboLevel==12)
				val+= 1.5;
			break;
		case 6://三带一
			val += -0.5;
			break;
		case 7://三带�?
			val += -0.5;
			break;
		case 8://炸弹
			val += 1;
			break;
		case 9:
		case 10:
			val -= 1;//tiao
			break;
		case 11://飞机
			val += -1.6+c.comboLevel*0.1;
			break;
		case 12://飞机带小�?
			val += -1.6+c.comboLevel*0.1;
			break;
		case 13://飞机带大�?
			val += -1.6+c.comboLevel*0.1;
			break;
		case 17://火箭
			val += 2;
			break;
	}
	return val;
}

double calValue(vector<CardCombo> v)
{
	double val = 0;
	auto it = v.begin();
	for (; it != v.end(); ++it) val+=getvalue(*it);
	val-=v.size()*0.1;
	return val;
}
namespace BotzoneIO
{
	using namespace std;
	void input()
	{
		// 璇诲叆杈撳叆锛堝钩鍙颁笂鐨勮緭鍏ユ槸鍗曡锛?
		string line;
		getline(cin, line);
		Json::Value input;
		Json::Reader reader;
		reader.parse(line, input);
		// 棣栧厛澶勭悊绗竴鍥炲悎锛屽緱鐭ヨ嚜宸辨槸璋併€佹湁鍝簺鐗?
		{
			auto firstRequest = input["requests"][0u]; // 涓嬫爣闇€瑕佹槸 unsigned锛屽彲浠ラ€氳繃鍦ㄦ暟瀛楀悗闈㈠姞u鏉ュ仛鍒?
			auto own = firstRequest["own"];
			auto llpublic = firstRequest["publiccard"];
			auto history = firstRequest["history"];
			for (unsigned i = 0; i < own.size(); i++)
				myCards.insert(own[i].asInt());
			for (unsigned i = 0; i < llpublic.size(); i++)
				landlordPublicCards.insert(llpublic[i].asInt());
			//if(llpublic.size()==0) what=1;
			if (history[0u].size() == 0)
				if (history[1].size() == 0)
					myPosition = 0; // 涓婁笂瀹跺拰涓婂閮芥病鍑虹墝锛岃鏄庢槸鍦颁富
				else
					myPosition = 1; // 涓婁笂瀹舵病鍑虹墝锛屼絾鏄笂瀹跺嚭鐗屼簡锛岃鏄庢槸鍐滄皯鐢?
				else
					myPosition = 2; // 涓婁笂瀹跺嚭鐗屼簡锛岃鏄庢槸鍐滄皯涔?
		}
		
		// history閲岀涓€椤癸紙涓婁笂瀹讹級鍜岀浜岄」锛堜笂瀹讹級鍒嗗埆鏄皝鐨勫喅绛?
		int whoInHistory[] = { (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT, (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT };
		
		int count=0;
		int turn = input["requests"].size();
		for (int i = 0; i < turn; i++)
		{
			// 閫愭鎭㈠灞€闈㈠埌褰撳墠
			auto history = input["requests"][i]["history"]; // 姣忎釜鍘嗗彶涓湁涓婂鍜屼笂涓婂鍑虹殑鐗?
			int howManyPass = 0;
			for (int p = 0; p < 2; p++)
			{
				int player = whoInHistory[p]; // 鏄皝鍑虹殑鐗?
				auto playerAction = history[p]; // 鍑虹殑鍝簺鐗?
				vector<Card> playedCards;
				for (unsigned _ = 0; _ < playerAction.size(); _++) // 寰幆鏋氫妇杩欎釜浜哄嚭鐨勬墍鏈夌墝
				{
					int card = playerAction[_].asInt(); // 杩欓噷鏄嚭鐨勪竴寮犵墝
					--currRes[card2level(playerAction[_].asInt())];
					playedCards.push_back(card);
				}
				whatTheyPlayed[player].push_back(playedCards); // 璁板綍杩欐鍘嗗彶
				cardRemaining[player] -= playerAction.size();
				
				if (playerAction.size() == 0)
				{
					howManyPass++;
					if (myPosition != 0 && player == 0)//涓嶆槸鍦颁富
					{
						if (whatTheyPlayed[2][i - 1].size() != 0)
						{
							CardCombo lack = CardCombo(whatTheyPlayed[2][i - 1].begin(), whatTheyPlayed[2][i - 1].end());
							switch (lack.comboTypeInt)
							{
								case 1:
									if (lack.comboLevel<8) enemyLack[1] = true;
									break;
								case 2:
									if (lack.comboLevel<8) enemyLack[2] = true;
									break;
								default:
									if (lack.comboLevel<11) enemyLack[lack.comboTypeInt] = true;
									
							}
							
						}
						else if (whatTheyPlayed[1][i - 1].size() != 0)
						{
							CardCombo lack = CardCombo(whatTheyPlayed[1][i - 1].begin(), whatTheyPlayed[1][i - 1].end());
							switch (lack.comboTypeInt)
							{
								case 1:
									if (lack.comboLevel<8) enemyLack[1] = true;
									break;
								case 2:
									if (lack.comboLevel<8) enemyLack[2] = true;
									break;
								default:
									if (lack.comboLevel<11) enemyLack[lack.comboTypeInt] = true;
									
							}
						}
						
						
					}
				}
				else
				{
					lastValidCombo = CardCombo(playedCards.begin(), playedCards.end());
				}
			}
			
			
			if (howManyPass == 2)
				lastValidCombo = CardCombo();
			
			if (i < turn - 1)
			{
				// 杩樿鎭㈠鑷繁鏇剧粡鍑鸿繃鐨勭墝
				auto playerAction = input["responses"][i]; // 鍑虹殑鍝簺鐗?
				vector<Card> playedCards;
				for (unsigned _ = 0; _ < playerAction.size(); _++) // 寰幆鏋氫妇鑷繁鍑虹殑鎵€鏈夌墝
				{
					int card = playerAction[_].asInt(); // 杩欓噷鏄嚜宸卞嚭鐨勪竴寮犵墝
					--currRes[card2level(playerAction[_].asInt())];
					myCards.erase(card); // 浠庤嚜宸辨墜鐗屼腑鍒犳帀
					playedCards.push_back(card);
				}
				whatTheyPlayed[myPosition].push_back(playedCards); // 璁板綍杩欐鍘嗗彶
				cardRemaining[myPosition] -= playerAction.size();
				
				if(myPosition==1&&count==1&&howManyPass == 2&&playerAction.size()==1) shi=1;
			}
			if(myPosition==1&&cardRemaining[2]==1) count=1;
			
		}
		updateDomain();//鏇存柊currDomain鏁扮粍
		/*
		if(io&&turn==1)
		{
			fstream fout;
			fout.open("/Users/xiaoxiong/Desktop/debug.txt", ios::app|ios::out);//ate
			if(myPosition==0) fout<<"{\"publiccard\":"<<landlordPublicCards<<",\"allocation\":["<<myCards<<',';
			if(myPosition==1) fout<<myCards<<',';
			if(myPosition==2) fout<<myCards<<"]}";
			fout.close();
		}
		 */
		
		 if(io)
		 {
		 fstream fout;
		 fout.open("/Users/xiaoxiong/Desktop/debug.txt", ios::app|ios::out);//ate
		 fout<<line<<endl;
		 fout.close();
		 }
		 
	}
	
	/**
	 * 杈撳嚭鍐崇瓥锛宐egin鏄凯浠ｅ櫒璧风偣锛宔nd鏄凯浠ｅ櫒缁堢偣
	 * CARD_ITERATOR鏄疌ard锛堝嵆short锛夌被鍨嬬殑杩唬鍣?
	 */
	template <typename CARD_ITERATOR>
	void output(CARD_ITERATOR begin, CARD_ITERATOR end)
	{
		Json::Value result, response(Json::arrayValue);
		for (; begin != end; begin++)
			response.append(*begin);
		result["response"] = response;
		
		Json::FastWriter writer;
		cout << writer.write(result) << endl;
	}
}

template <typename CARD_ITERATOR>
CardCombo zhudong(CARD_ITERATOR begin, CARD_ITERATOR end)
{
	CardCombo c(begin,end);
	if(c.comboType != CardComboType::INVALID) return c;//一手出完
	vector<vector<CardCombo>> all = fenpai(begin, end);
	CardCombo maxcombo;
	double maxvalue = -1000;
	for (auto i = all.begin(); i != all.end(); i++)//i代表每种分牌方式
	{
		if(notbest(*i)<=1) {for(auto it=i->begin();it!=i->end();it++) if(it->isbest()) return *it; what=1;}//只有一手不是最大
		double value = 0, minvalue = 1000;
		CardCombo max;
		for (auto j = i->begin(); j != i->end(); j++)// if(j->cards.size()==0) {what=1;} else
		{
			double val = getvalue(*j);
			double tmp = 0;
			//if(j->isbest()) tmp+=0.1;//tiao1
			if (j->comboTypeInt == 1 || j->comboTypeInt == 2)
				//单张or对子
			{
				tmp -= (double)j->comboLevel / 200;
				if ((myPosition == 0 && (cardRemaining[1] == j->cards.size() || cardRemaining[2] == j->cards.size()))
					|| (myPosition != 0 && cardRemaining[0] == j->cards.size()))
					if(!j->isbest()) tmp -= (double)(16-j->comboLevel);
			}
			else if (j->comboTypeInt == 11 || j->comboTypeInt == 5)
				//三不带or飞机不带
			{
				tmp += 0.3;
			}
			else if (j->comboTypeInt == 9 || j->comboTypeInt == 10)
				//四带二
			{
				tmp -= 10;//tiao
			}
			else if (j->comboTypeInt == 3 || j->comboTypeInt == 4 || j->comboTypeInt == 6 || j->comboTypeInt == 7 || j->comboTypeInt == 12 || j->comboTypeInt == 13)
				//三带or飞机带翅膀
			{
				tmp += 0.5;
			}
			/*
			 if((myPosition==0&&(cardRemaining[1]<=3||cardRemaining[2]<=3))||(myPosition!=0&&cardRemaining[0]<=3))
			 //对手剩的牌过少
			 {
			 tmp -= (double)(16-j->comboLevel)/10;
			 }
			 */
			value += val;
			if (val - tmp < minvalue)
			{
				minvalue = val - tmp;
				max = *j;
			}
			/*
			 if(j->comboLevel == 11)
			 {
			 tmp -= 0.5;
			 }
			 else if(j->comboLevel == 12)
			 tmp -= 3;
			 */
		}
		if (value - minvalue - i->size()*0.1>maxvalue)
		{
			maxvalue = value - minvalue - i->size()*0.1;
			maxcombo = max;
			//std::cout<<*i<<std::endl;
		}
	}
	if(myPosition==1&&cardRemaining[2]==1&&!shi)
	{
		auto second = begin;
		second++;
		CardCombo cc(begin, second); // 那么就出第一张牌……
		if(!cc.isbest()) return cc;
	}
	return maxcombo;
}
int main(int argc, const char * argv[])
{
	//if(argc==2) io=1;
	BotzoneIO::input();
	insert_all_card();
	// 鍋氬嚭鍐崇瓥锛堜綘鍙渶淇敼浠ヤ笅閮ㄥ垎锛?
	
	// findFirstValid 鍑芥暟鍙互鐢ㄤ綔淇敼鐨勮捣鐐?
	CardCombo myAction = lastValidCombo.findFirstValid(myCards.begin(), myCards.end());
	//std::cout<<myAction;
	// 鏄悎娉曠墝
	assert(myAction.comboType != CardComboType::INVALID);
	
	assert(
		   // 鍦ㄤ笂瀹舵病杩囩墝鐨勬椂鍊欒繃鐗?
		   (lastValidCombo.comboType != CardComboType::PASS && myAction.comboType == CardComboType::PASS) ||
		   // 鍦ㄤ笂瀹舵病杩囩墝鐨勬椂鍊欏嚭鎵撳緱杩囩殑鐗?
		   (lastValidCombo.comboType != CardComboType::PASS && lastValidCombo.canBeBeatenBy(myAction)) ||
		   // 鍦ㄤ笂瀹惰繃鐗岀殑鏃跺€欏嚭鍚堟硶鐗?
		   (lastValidCombo.comboType == CardComboType::PASS && myAction.comboType != CardComboType::INVALID)
		   );
	
	// 鍐崇瓥缁撴潫锛岃緭鍑虹粨鏋滐紙浣犲彧闇€淇敼浠ヤ笂閮ㄥ垎锛?
	
	BotzoneIO::output(myAction.cards.begin(), myAction.cards.end());
	//return myAction.getWeight();
}