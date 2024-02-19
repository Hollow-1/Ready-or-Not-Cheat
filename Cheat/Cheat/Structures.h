#pragma once

using namespace std;

class TArray
{
public:
	bool IsValid() const {
		return addr != NULL && count <= max;
	}

	ULONG_PTR addr;
	uint32 count;
	uint32 max;
};

class FName
{
public:

    uint32 index;
    uint32 number;

    FName() : index(0), number(0) {}
    FName(uint32 index, uint32 number) : index(index), number(number) {}

    //Ϊ����ӽ� std::map�� ��Ҫ���� < �������mapֻʹ�� < �Ϳ��Խ��и��ֱȽ�
    bool operator<(const FName& other)const {
        if (index != other.index) {
            return index < other.index;
        }
        return number < other.number;
    }

    string GetString() {
        string name = GetStringWithoutNumber();
        if (number != 0) {
            name = name + "_" + std::to_string(number - 1);
        }
        return name;
    }
    //����ʵ���� Function.h
    string GetStringWithoutNumber();

    static string XorCypher(string input) {
        INT32 key = (INT32)input.size();
        string output = input;

        for (size_t i = 0; i < input.size(); i++)
            output[i] = input[i] ^ key;

        return output;
    }



};