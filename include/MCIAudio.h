//Author:zmcj21
//Date:2021/12/29
//License:MIT
//Repo:https://github.com/zmcj21/MCIAudio

//本项目基于以下文档开发:
//Microsoft MCI Document:https://docs.microsoft.com/en-us/windows/win32/multimedia/mci

#pragma once

//Enable Unicode
#if !defined(UNICODE)
#define UNICODE
#endif
//Reduces the size of the Win32 header files by excluding some of the less frequently used APIs
#define WIN32_LEAN_AND_MEAN
//include Windows.h first
#include <Windows.h>
//Provide MCI interface, mmsystem header file depends on Windows.h
#include <mmsystem.h>
//C++ headers:
#include <string>

namespace MCIAudio
{
    class Audio
    {
    public:
        static std::wstring MCIAlias;

        static int MCIAliasCounter;

    public:
        static void MCISendString(const std::wstring& s);

        static std::wstring MCISendStringEx(const std::wstring& s);

        static std::wstring MCIGetErrorString(MCIERROR err);

        //tool functions:
    public:
        static std::wstring _GetShortPathName(const std::wstring& path);

        static std::wstring _FormatLastErrorMsg();

        static std::wstring _GetFileExtension(const std::wstring& path);

        static std::wstring _GetDirectoryPath();

    public:
        std::wstring path;      //Absolute path
        std::wstring shortPath; //Short path
        std::wstring extension; //Extension of file
        std::wstring alias;     //Unique alias

        int totalMilliSecond;   //Total milliSecond of this audio
        int minute;             //Minute part of this audio
        int second;             //Second part of this audio
        int milliSecond;        //MilliSecond part of this audio

    public:
        Audio();

        Audio(const std::wstring& path);

        ~Audio();

        void Open(const std::wstring& path);

        void Close();

        void Play(bool repeat = false, bool wait = false);

        void PlayEx(int from, int to, bool repeat = false, bool wait = false);

        void Stop();

        void Pause();

        void Resume();
    };
}

#ifdef MCI_AUDIO

//link input
#pragma comment(lib, "winmm.lib")

//Each string that MCI returns, whether data or an error description, can be a maximum of 128 characters.
#ifndef MCI_ERR_BUFFER_SIZE
#define MCI_ERR_BUFFER_SIZE 128
#endif

//just set it to 128 :)
#ifndef MCI_MSG_BUFFER_SIZE
#define MCI_MSG_BUFFER_SIZE 128
#endif

#ifndef MCI_UNKNOWN_EXCEPTION
#define MCI_UNKNOWN_EXCEPTION "Unknown MCI Exception"
#endif

namespace MCIAudio
{
    std::wstring Audio::MCIAlias = L"MCI_ALIAS_";

    int Audio::MCIAliasCounter = 1;

    void Audio::MCISendString(const std::wstring& s)
    {
        MCIERROR err = mciSendString(s.c_str(), nullptr, 0, nullptr);
        if (err != 0)
        {
            std::wstring err_msg = MCIGetErrorString(err);
            throw err_msg;
        }
    }

    std::wstring Audio::MCISendStringEx(const std::wstring& s)
    {
        WCHAR buf[MCI_MSG_BUFFER_SIZE];
        MCIERROR err = mciSendString(s.c_str(), buf, MCI_MSG_BUFFER_SIZE, nullptr);
        if (err == 0)
        {
            return std::wstring(buf);
        }
        else
        {
            std::wstring err_msg = MCIGetErrorString(err);
            throw err_msg;
        }
    }

    std::wstring Audio::MCIGetErrorString(MCIERROR err)
    {
        WCHAR buf[MCI_ERR_BUFFER_SIZE];
        bool suc = mciGetErrorString(err, buf, MCI_ERR_BUFFER_SIZE);
        if (suc)
        {
            return std::wstring(buf);
        }
        else
        {
            throw MCI_UNKNOWN_EXCEPTION;
        }
    }

    std::wstring Audio::_GetShortPathName(const std::wstring& path)
    {
        WCHAR buf[MAX_PATH];
        bool suc = GetShortPathName(path.c_str(), buf, MAX_PATH);
        if (suc)
        {
            return std::wstring(buf);
        }
        else
        {
            throw _FormatLastErrorMsg();
        }
    }

    std::wstring Audio::_FormatLastErrorMsg()
    {
        WCHAR buf[MAX_PATH];
        bool suc = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, MAX_PATH, nullptr);
        if (suc)
        {
            return std::wstring(buf);
        }
        else
        {
            throw MCI_UNKNOWN_EXCEPTION;
        }
    }

    std::wstring Audio::_GetFileExtension(const std::wstring& path)
    {
        std::wstring extension;

        size_t pos = path.find_last_of(L'.');
        if (pos == std::wstring::npos)
        {
            return extension;
        }

        extension = path.substr(pos + 1);
        return extension;
    }

    std::wstring Audio::_GetDirectoryPath()
    {
        std::wstring path;

        WCHAR buffer[MAX_PATH];
        ::GetModuleFileName(NULL, buffer, MAX_PATH);

        path = buffer;
        int index = path.find_last_of(L'\\');
        std::wstring directory_path = path.substr(0, index + 1);

        return directory_path;
    }

    Audio::Audio()
    {
    }

    Audio::Audio(const std::wstring& path)
    {
        Open(path);
    }

    Audio::~Audio()
    {
        Close();
    }

    void Audio::Open(const std::wstring& path)
    {
        //必须将路径转成ShortPath, 否则MCI可能无法识别并出现错误代码263
        std::wstring shortPathName = _GetShortPathName(path);
        std::wstring alias = MCIAlias + std::to_wstring(MCIAliasCounter++);

        MCISendString(L"open " + shortPathName + L" alias " + alias);

        this->path = path;
        this->shortPath = shortPathName;
        this->extension = _GetFileExtension(path);
        this->alias = alias;

        std::wstring length_s = MCISendStringEx(L"status " + alias + L" length");
        this->totalMilliSecond = _wtoi(length_s.c_str());
        this->minute = totalMilliSecond / 1000 / 60;
        this->second = totalMilliSecond / 1000 - minute * 60;
        this->milliSecond = totalMilliSecond % 1000;
    }

    void Audio::Close()
    {
        MCISendString(L"close " + this->alias);
    }

    void Audio::Play(bool repeat, bool wait)
    {
        PlayEx(0, this->totalMilliSecond, repeat, wait);
    }

    void Audio::PlayEx(int from, int to, bool repeat, bool wait)
    {
        std::wstring cmd = L"play " + this->alias;

        //from x ms to y ms
        cmd += L" from " + std::to_wstring(from) + L" to " + std::to_wstring(to);

        //NOTICE:if play .wav music, repeat is useless. If file is .wav and repeat is on, it will fail. Seems it's a bug in MCI.
        if (repeat && this->extension.compare(L".wav") != 0)
        {
            cmd += L" repeat";
        }
        //wait will block the thread.
        if (wait)
        {
            cmd += L" wait";
        }

        MCISendString(cmd);
    }

    void Audio::Stop()
    {
        std::wstring cmd = L"stop " + this->alias;
        MCISendString(cmd);
    }

    void Audio::Pause()
    {
        std::wstring cmd = L"pause " + this->alias;
        MCISendString(cmd);
    }

    void Audio::Resume()
    {
        std::wstring cmd = L"resume " + this->alias;
        MCISendString(cmd);
    }

}

#endif
