//Copyright 2015 The Salmon Censorship Circumvention Project
//
//This file is part of the Salmon Client (Windows).
//
//The Salmon Client (Windows) is free software; you can redistribute it and / or
//modify it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//GNU General Public License for more details.
//
//The full text of the license can be found at:
//http://www.gnu.org/licenses/gpl.html

#include "stdafx.h"
#include "Resource.h"

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include "salmon_globals.h"
#include "hwnds.h"

#include "salmon_utility.h"
#include "localization.h"


const int NUM_DIR_SERVER_MESSAGES = 22;
const int NUM_LANGUAGES_SUPPORTED = 3;
const WCHAR* dirMsgStrings[NUM_DIR_SERVER_MESSAGES][NUM_LANGUAGES_SUPPORTED] =
{
	//pop-up message boxes that come from the directory server
	//$0
	{
		L"Error! The directory server sent an unknown message code. This might be an error in the server, or you might not have the newest version of this program.",
		L"错误!无法解读中央服务器信息代码。这可能是服务器错误导致，也有可能是你的客户端不是最新版本。",
		L" خطا! کدارسال شده توسط سرور دایرکتوری، شناخته شده نیست. این مشکل ممکن است به خاطر خطا در سرور بوده، یا ممکن است نرم افزار شما جدیدترین نسخه نباشد"
	},
	//$1
	{
L"Error! This email address is already registered. If you just tried to register with this same address \
and the attempt seemed to fail, then probably the server views you as successfully registered, but your \
client program doesn't realize it. In that case, just exit Salmon, start Salmon again, and log in as an existing user.",
L"错误!邮箱地址已经被注册。如果你已经注册，请尝试退出客户端载重新打开。",
		L".خطا! آدرس ایمیل واردشده قبلاً ثبت شده است. اگر تازه با این ایمیل ثبت نام کرده اید و تلاشتان ناموفق بوده است به احتمال زیاد سرور شما را میشناسد اما کلاینت هنوز متوجه نشده است. در این صورت از برنامه ماهی آزاد خارج شوید، برنامه را دوباره باز کنید و به عنوان کاربر ثبت شده وارد سیستم شوید. "
	},
	//$2 (UNUSED: INSECURE) This social network account has already been used to create a Salmon account.
	{
		L"Users should never see this message",
		L"用户不该看到此消息。",
		L"این حساب شبکه اجتماعی قبلاً برای ایجاد حساب ماهی آزاد استفاده شده است"
	},
	//$3
	{
		L"You've already posted the post you said you would post in the future. Please start registration over with a new post, and don't post the post until prompted!",
		L"你已经发出你所声明的状态。请用一条新的状态重新开始注册。不要发出状态直到我们让你这样做!",
		L"شما پستی که گفته بودید را قبلا ارسال کرده اید. لطفاً ثبت نام را دوباره با یک پست جدید آغاز نمایید. لطفاً پست را تا موقعی که از شما درخواست نکردیم ارسال نکنید"
	},
	//$4
	{
		L"Couldn't access your social network account. Please ensure your account is publically viewable, and start registration over. (You can make it private again once registered.)",
		L"无法检测到你的社交账号。请重新注册。",
		L".خطا! دسترسی به حساب شبکه اجتماعی شما به شکل عمومی ممکن نیست. لطفا حساب کاربری شبکه اجتماعی خود را قابل دسترسی کنید و مراحل ثبت نام را مجدداً آغاز نمایید. (پس از پایان مراحل ثبت نام می توانید یه حالت خصوصی برگردانید.)"
	},
	//$5
	{
		L"Ok! Registration is half-done. Now go post the post.",
		L"OK!注册过程已经完成了一半。现在请发出你声明的状态。",
		L"خیلی خوب! نیمی از مراحل ثبت نام انجام شده است. حالا یک پست جدبد بفرست! "
	},
	//$6
	{
		L"We don't have a record of your email having started the registration process.",
		L"我们没有记录到你的账号开始了注册过程。",
		L"اطلاعاتی ایمیل شما که ثبت نام کرده باشید موجود نیست"
	},
	//$7
	{
		L"Ok! You are registered. Click Connect to connect (this first time may take a while).",
		L"OK!你已经注册成功。点击\"连接\"，初次连接可能会耗时较长。",
		L" .خیلی خوب! ثبت نام شما با موفقیت انجام شده است. دکمه اتصال را برای وصل شدن کلیک نمایید. اولین بار ممکن است کمی طول بکشد تا وصل شود."
	},
	//$8
	{
L"We didn't see the post on your social network page. Be sure you posted \
exactly what you told us (check spelling). If you definitely posted it, \
it might just need a little more time to become visible - try again in a few seconds.",
L"我们没有在你的社交账号上检测到你声明的状态。请确保你发的跟你声明的完全一致（检查拼写）。如果你确定你已经发出了，那可能需要等待一段时间我们才能看到。请等几秒再试。",
		L"پست شما در صفحه شبکه اجتماعی شما پیدا نشد. اطمینان حاصل کنید دقیقاً همان پستی که بیان کردید را پست کردید (اشتباهات نوشتاری را بررسی کنید). اگر مطمئناً این پست را پست کردید، ممکن است کمی طول بکشد که قابل مشاهده باشد. لطفاً چند ثانیه بعد دوباره امتحان کنید."
	},
	//$9
	//NOTE this one has a special format: $9$thecode
	{
		L"Recommendation code not found! Check for typos. Has it already been used? The code you gave us: ",
		L"邀请码未找到。请检查拼写。检查是否已被使用。你输入的邀请码是：",
		L"کد توصیه یافت نشد! اشتباهات نوشتاری را بررسی کنید. آیا قبلاً از این کد استفاده شده است؟ این کدی که شما وارد کردید می باشد؟: "
	},
	//$10
	{
		L"Recommendation successful! You are now a registered, trusted user.",
		L"邀请成功。你现在是一个受信任的注册用户。",
		L"توصیه با موفقیت انجام شد! شما اکنون یک کاربر ثبت شده و معتمد می باشید"
	},
	//$11
	{
L"Your account is permanently banned. If you aren't helping a government block servers, \
then we're sorry. Unfortunately, unless we had many more VPN servers to burn through, it's \
impossible to be 100% accurate with our bans. If you want to get a new account, \
get a recommendation code from a highly trusted user.",
L"你的账号已经被永久封禁。如果你并没有帮助政府破坏我们的系统，那我们表示抱歉。除非我们有足够多的VPN服务器，不然在封禁用户时，我们无法保证百分之百的正确率。如果你想要一个新的账号，从高信任等级的用户处获得一个邀请码。",
		L"حساب شما به طور دائمی بسته شده است. اگر شما به یک گروه خاص برای مسدود کردن دسترسی به سرورها کمک نمی کنید، برای شما متاسفیم. متاسفانه غیرممکن است که عملیات مسدود کردن ما 100% دقیق باشد، مگر اینکه تعداد بسیار زیادی از وی پی ان هایمان را برای این کار استفاده می کردیم. اگر می خواهید حساب جدیدی باز کنید، باید کد توصیه از یک کاربر با اعتیار بالا دریافت کنید."
	},
	//$12
	{
		L"Sorry! You can't recommend friends until you are highly trusted, which may take over a month.",
		L"抱歉!你的信任等级不够高，一般需要多于一个月的使用时间才能符合获取邀请码的条件。",
		L"متاسفم! تا موقعی که درجه اعتبار شما بالا نباشد نمی توانید دوستان خود را توصیه کنید"
	},
	//$13
	//NOTE this one has a special format: $13$thecode
	{
		L"Your previous code has not yet been used and is still active. Please use that one: ",
		L"你的前一个邀请码还未被使用，请使用这个：",
		L"کد قبلی شما هنوز استفاده نشده است و فعال است. لطفا آن کد را استفاده کنید: "
	},
	//$14
	{
		L"Sorry! You can only recommend once per month.",
		L"抱歉!你一个月只能获得一个邀请码。",
		L"متاسفم! شما فقط یک بار در ماه می توانید توصیه نمایید"
	},
	//$15
	{
		L"Sorry! You can only recommend once per day.",
		L"抱歉!你一天只能获得一个邀请码。",
		L"متاسفم! شما فقط یک بار در روز می توانید توصیه نمایید"
	},
	//$16
	{
		L"This recommendation doesn't provide more trust than we already have for you! Give the code to someone who isn't already highly trusted.",
		L"这个邀请码不能提高你的信任等级。请把这个邀请码给信任度更低的用户。",
		L"این توصیه درجه اطمینان شما را بیشتر از قبل نمی کند! کد را به کسی بدهید که درجه اطمینانش از قبل آنچنان بالا نیست"
	},
	//$17
	{
		L"Recommendation successful!",
		L"邀请成功!",
		L"عملیات توصیه موفق آمیز بوده است."
	},
	//$18 TODO TODO translations are out of date
	{
L"Salmon has run out of new servers at your trust level! We are unable to give \
you a new server until we get more volunteers. Now trying to fall back to a VPN Gate server. \
Even if there is a VPN Gate server available, please encourage your friends who live in uncensored \
countries to run Salmon servers: \
Salmon servers are much less vulnerable to being blocked than VPN Gate servers.\n\
(https://salmon.cs.illinois.edu/)\n\n\
Note that your currently assigned Salmon servers might just be temporarily offline, not blocked.",
L"Salmon没有足够的与你的信任等级相对应的服务器!我们现在无法给你分配一个新的服务器，直到我们获得更多服务器。如果你有朋友身处不进行网络审查的国家，请鼓励他做志愿者提供服务器。一些已经分配给你的服务器可能暂时下线，请稍后再试。\n\n\
Now trying to fall back to a VPN Gate server.",
L"برنامه ماهی آزاد دیگر سروری برای درجه اعتماد شما ندارد! متاسفانه ما تا زمانی که داوطلبان بیشتری نداشته باشیم نمی توانیم سرور جدیدی به شما بدهیم. اگر دوستی در کشورهای سانسورنشده دارید، لطفاً آنها را تشویق کنید که داوطلب بشوند - https://salmon.cs.illinois.edu \n\
همچنین احتمال دارد بعضی از سرورهای به کار گرفته شده برای شما ممکن است آفلاین باشند.لطفاً بعداً دوباره امتحان نمایید    \n\n\
Now trying to fall back to a VPN Gate server."
	},
	//$19
	{
		L"Sorry, there are currently no servers you can access. You may have come under suspicion if many of the servers you were given got blocked. Wait a few days and try again.",
		L"抱歉，现在没有服务器可以提供给你。你有可能被怀疑，由于之前提供给你的很多服务器被屏蔽。请等待几天后再试。",
		L"متاسفم! سرور بیشتری برای دسترسی وجود ندارد. ممکن است تصور کنید بسیاری از سرورهایی که در اختیار شما گذاشته شده اند بلاک شده باشند. چند روزی منتظر بمانید و دوباره امتحان نمایید"
	},
	//$20
	{
		L"We could not access your social network profile, because it is not set to be publicly viewable. Please make it public and start registration over.",
		L"我们无法访问你的社交账号主页，由于你的隐私设置不是所有人可见。请将其设置为公开后重试注册。",
		L"دسترسی‌ به حساب کاربری(پروفایل) شخصی‌ شما به دلیل تنظیمات فعلی‌ آن امکان پذیر نبود.لطفا تنظیمات حساب کاربری(پروفایل) خود را به عمومی‌(پابلیک) تغییر داده و عملیات ثبت نام را مجددا شروع کنید."
	},
	//$21
	{
		L"Your Renren account is not using the Renren Timeline. Please upgrade to Timeline, and start registration over.",
		L"你的人人账户未使用时间线功能。请开启时间线功能后重试注册。",
		L"حساب رنرن شما هم اکنون از جدول زمانی‌ (تایم‌لاین) استفاده نمیکند. لطفا حساب خود را به جدول زمانی‌ (تایم‌لاین) ارتقا داده و عملیات ثبت نام را مجددا شروع کنید."
	}
};

bool checkIfSuccessfulStartReg(const char* asciiStr)
{
	//we might eventually have message indices > 49, so we can't just check the first 2 characters.
	//on the other hand, if we just did a strcmp, there could be a stray \n or something to mess us up.
	if (strlen(asciiStr) > 2 && asciiStr[2] >= '0' && asciiStr[2] <= '9')
		return false;

	return (strncmp(asciiStr, "$5", 2) == 0);
}

void localizeDirServMsgBox(const char* asciiStr, const WCHAR* boxTitle)
{
	//special handling for message 9, which has to contain a rec code from the dir.
	//NOTE this one has a special format: $9$thecode
	if (strlen(asciiStr) > 3 && !strncmp(asciiStr, "$9$", 3))
	{
		WCHAR* wideCodeStr = new WCHAR[strlen(asciiStr) + 1];
		mbstowcs(wideCodeStr, asciiStr + 3, strlen(asciiStr) + 1);
		WCHAR* wholeStr = new WCHAR[wcslen(wideCodeStr) + wcslen(dirMsgStrings[9][gChosenLanguage])+1];
		wcscpy(wholeStr, dirMsgStrings[9][gChosenLanguage]);
		wcscat(wholeStr, wideCodeStr);
		MessageBox(NULL, wholeStr, boxTitle, MB_OK);
		delete wideCodeStr;
		delete wholeStr;
	}
	//special handling for message 13, which has to contain a rec code from the dir.
	//NOTE this one has a special format: $13$thecode
	else if (strlen(asciiStr) > 4 && !strncmp(asciiStr, "$13$", 4))
	{
		WCHAR* wideCodeStr = new WCHAR[strlen(asciiStr) + 1];
		mbstowcs(wideCodeStr, asciiStr + 4, strlen(asciiStr) + 1);
		WCHAR* wholeStr = new WCHAR[wcslen(wideCodeStr) + wcslen(dirMsgStrings[13][gChosenLanguage]) + 1];
		wcscpy(wholeStr, dirMsgStrings[13][gChosenLanguage]);
		wcscat(wholeStr, wideCodeStr);
		MessageBox(NULL, wholeStr, boxTitle, MB_OK);
		delete wideCodeStr;
		delete wholeStr;
	}

	//some not-to-be-localized message. could be some add-on message in the future, but currently, this is used to display rec. codes.
	else if (asciiStr[0] != '$' || asciiStr[1] < '0' || asciiStr[1] > '9')
	{
		WCHAR* wideStr = new WCHAR[strlen(asciiStr) + 1];
		mbstowcs(wideStr, asciiStr, strlen(asciiStr) + 1);
		MessageBox(NULL, wideStr, boxTitle, MB_OK);
		delete wideStr;
	}
	else
	{
		char errCodeBuf[10];
		int errCharInd = 0;
		for (const char* curErrChar = asciiStr + 1; *curErrChar >= '0' && *curErrChar <= '9' && errCharInd<9; curErrChar++, errCharInd++)
			errCodeBuf[errCharInd] = *curErrChar;
		errCodeBuf[errCharInd] = 0;

		int strCode = atoi(errCodeBuf);
		if (strCode >= NUM_DIR_SERVER_MESSAGES)
		{
			WCHAR* wideStr = new WCHAR[strlen(asciiStr) + 1];
			mbstowcs(wideStr, asciiStr, strlen(asciiStr) + 1);
			//NOTE: 0th string is "this message code is undefined"
			MessageBox(NULL, dirMsgStrings[0][gChosenLanguage], wideStr, MB_OK);
			delete wideStr;
			return;
		}
		else
			MessageBox(NULL, dirMsgStrings[strCode][gChosenLanguage], boxTitle, MB_OK);
	}
}


const WCHAR* allStrings[87][NUM_LANGUAGES_SUPPORTED] =
{
	//pop-up message boxes
	//COULDNT_READ_CONFIG_FILE
	{
		L"Could not read Salmon config file.",
		L"无法读取Salmon配置文件。",
		L"خطا در خواندن فایل تنظیمات ماهی آزاد"
	},
	//COULDNT_CONNECT_WILL_needServer
	{
		L"Couldn't connect to any of your current servers. We will have to ask the directory server for a new one, which might take over a minute. Click OK to continue.",
		L"无法连接到现有服务器。我们需要向中央服务器获取一个新的，这可能会需要一分钟。点击OK继续。",
		L"نمی توان به هیچ کدام از سرورها متصل شد. ما بایستی از دایرکتوری سرور، سرور جدیدی درخواست کنیم که ممکن است یک دقیقه طول بکشد. جهت ادامه بر روی دکمه اکی کلیک نمایید"
	},
	//SORRY_NO_SERVERS_AVAILABLE
	{
		L"Sorry, no servers available to connect to.",
		L"抱歉，没有可用的服务器。",
		L"متاسفم! سروری برای اتصال موجود نیست"
	},
	//FAILED_TO_SEND_EMAIL
	{
L"Failed to send email to directory server. Perhaps there was a typo?\n\n\
If you just created the email account, check its inbox:\n\
the account might need to be verified.\n\n\
Otherwise, check your internet connection.",
L"无法向中央服务器发送信息。请检查拼写。\n\n如果你使用的是新建邮箱，请检查收件箱：你可能需要验证你的邮箱。\n\n否则，请检查你的密码或者网络连接。",

L" \
عدم موفقیت در ارسال ایمیل به دایرکتوری سرور. ممکن است خطای تایپی داشته باشید\n\n\
اگر حساب کاربریتان را تازه باز کرده اید اینباکس اش را چک کنید\n\
احتمالا حساب باید تایید  شود\n\n\
اگر هیچ یک از این ها نبود چک کنید آیا به اینترنت وصل هستید؟\
"
	},
	//FAILED_TO_READ_EMAIL
	{
		L"Could not read mail from your email account.",
		L"无法从你的账号读取电子邮件。",
		L"خطا در خواندن نامه ها از حساب ایمیل شما"
	},
	//SOFTETHER_EXE_DOESNT_EXIST
	{
		L"The following file does not exist. SoftEther must be installed to that directory for Salmon to work:\n",
		L"以下文件不存在。SoftEther必须安装在以下文件夹以便Salmon正常工作：\n",
		L"فایل مقابل موجود نیست. برای کارکردن برنامه ماهی آزاد، برنامه سافت ایتر باید در آن دایرکتوری نصب شود"
	},
	//COULDNT_POPEN_PING (NOTE: obsolete, but can be left as-is)
	{
		L"Could not _popen ping! Windows is not behaving as expected...",
		L"无法打开ping!windows表现得不像期望那样...",
		L"خطا در اجرای دستور پینگ! ویندوز آن طور که انتظار می رفت رفتار نمی کند ..."
	},
	//DIRSERV_GAVE_MALFORMED_RESPONSE_TO_LOGIN
	{
		L"Directory server returned an incorrectly formatted reply to our login request.",
		L"中央服务器对登陆要求返还的回复格式不正确。",
		L" دایرکتوری سرور در پاسخ به درخواست ورود ما پاسخی با فرمت اشتباه ارسال کرده است"
	},
	//ABORT_REG_NO_RESPONSE
	{
		L"No response received from directory server. Registration aborted.",
		L"中央服务器无响应。注册取消。",
		L".پاسخی از دایرکتوری سرور دریافت نشده است. ثبت نام بی نتیجه است"
	},
	//ABORT_LOGIN_NO_RESPONSE
	{
		L"No response received from directory server. Login aborted.",
		L"中央服务器无响应。登陆取消。",
		L"پاسخی از دایرکتوری سرور دریافت نشده است. ورود بی نتیجه است"
	},
	//NO_RESPONSE_FROM_DIR
	{
		L"No response received from directory server.",
		L"中央服务器无响应。",
		L"پاسخی از دایرکتوری سرور دریافت نشده است"
	},
	//MUST_SELECT_FB_OR_RENREN
	{
		L"Please select Facebook.",//COMMENTRENRENL"Please select either Facebook or Renren.",
		L"请选择Facebook",//COMMENTRENRENL"请选择Facebook或者人人。",
		L"لطفاً فیسبوک را انتخاب نمایید"
	},
	//HAVE_YOU_POSTED_YET
	{
		L"Have you posted the post on your wall yet?",
		L"你已经发出了状态吗？",
		L"آیا تا الان پست موردنظر را روی دیوار خود پست کرده اید؟"
	},
	//PLEASE_POST_NOW
	{
		L"Please post it on your wall now.",
		L"现在请发出状态。",
		L"لطفاً پست را در دیوار خود پست نمایید"
	},












	//static text boxes
	//VPN_STATUS_CONNECTED
	{
		L"VPN status: Connected!",
		L"VPN状态：已连接",
		L"وضعیت وی پی ان: اتصال انجام شده است"
	},
	//VPN_STATUS_DISCONNECTED
	{
		L"VPN status: DISCONNECTED",
		L"VPN状态：连接断开",
		L"وضعیت وی پی ان: اتصال قطع شده است"
	},
	//VPN_STATUS_CONNECTING
	{
		L"VPN status: connecting...",
		L"VPN状态：正在连接...",
		L"وضعیت وی پی ان: در حال اتصال"
	},
	//FIRST_TIME_USER_Q
	{
		L"First time user?",
		L"第一次使用？",
		L"آیا کاربر جدیدی هستید؟"
	},
	//EXISTING_USER_Q
	{
		L"Existing user?",
		L"已注册用户？",
		L"آیا قبلا کاربر بوده اید؟"
	},
	//PROMPT_EMAIL_ADDR
	{
		L"Gmail, Hotmail/Outlook, or Yahoo address:",
		L"Gmail, Hotmail/Outlook 或 Yahoo 邮箱:",
		L"آدرس ایمیل گوگل، یاهو یا هاتمیل خود را وارد کنید  "
	},
	//PROMPT_EMAIL_ADDR_VERBOSE
	{
L"Enter a gmail, hotmail/outlook, or yahoo email address you own. You can use your personal account, \
or make a new one. If making a new one, you should follow the process for verifying the \
email account, or the emails this program sends might get silently blocked.",
L"输入一个你的Gmail, Hotmail/Outlook 或 Yahoo邮箱。可以使用你的个人或者新建邮箱。请认证你的邮箱，否则客户端所发出的邮件可能会被拦截。",
L"لطفاً آدرس  جیمیل یا یاهو یا هاتمیل خود را به شکل کامل وارد نمایید. شما می توانید از آدرس شخصی خود استفاده کنید یا برای این منظور آدرس جدیدی را ایجاد نمایید. "
	},
	//PROMPT_EMAIL_PASSWORD
	{
		L"Email account password:",
		L"邮箱密码：",
		L"پسوورد ایمیل:"
	},
	//PROMPT_EMAIL_PW_VERBOSE
	{
		L"The email account's password:",
		L"邮箱账号密码:",
		L"پسوورد حساب ایمیل مذکور:"
	},
	//TRUST_LVL_ORDINARY
	{
		L"Trust level: ordinary",
		L"信任等级：普通",
		L"درجه اعتماد: معمولی"
	},
	//TRUST_LVL_LOW
	{
		L"Trust level: low",
		L"信任等级：低",
		L"درجه اعتماد: کم"
	},
	//TRUST_LVL_HIGH
	{
		L"Trust level: high",
		L"信任等级：高",
		L"درجه اعتماد: زیاد"
	},
	//TRUST_LVL_HIGHEST
	{
		L"Trust level: highest",
		L"信任等级：很高",
		L"درجه اعتماد: بسیار زیاد"
	},
	//TRUST_LVL_MAX
	{
		L"Trust level: MAX",
		L"信任等级：最高",
		L"درجه اعتماد: بالاترین حد"
	},
	//BOOST_TRUST_W_CODE
	{
		L"Boost trust w/ rec. code",
		L"使用邀请码来提高信任等级",
		L"افزایش اعتماد با کد توصیه"
	},
	//REC_CODE_FROM_FRIEND
	{
		L"Recommendation code gotten from your friend:",
		L"从你朋友处获得的邀请码：",
		L"کد توصیه دریافت شده از دوست شما:"
	},
	//ENTER_POST_INSTRUCTIONS
	{
		L"Enter a post that you have not yet posted on your wall. After clicking \"Submit Registration\", you will be instructed to post it to your wall.",
		L"输入一个你未曾发出的新状态。点击\"提交注册\"之后，你会被告知何时发出状态。",
		L"پستی را که تا به حال روی دیوار خود نفرستاده اید پست نمایید. بعد از کلیک کردن بر روی دکمه *انجام ثبت نام*، از شما درخواست خواهد شد آن را بر روی دیوار خود پست نمایید"
	},
	//RENREN_STR
	{
		L"Renren",
		L"人人",
		L"رن رن"
	},
	//FACEBOOK_ID_INSTRUCTIONS
	{
		L"Facebook ID: if your profile URL is www.facebook.com/john.doe.123,\nenter john.doe.123",
		L"Facebook ID：假设你的账户主页地址是\"www.facebook.com/john.doe.123\"\n输入\"john.doe.123\"",
		L"آیدی فیسبوک: آیدی شما عبارت بعد از عبارت  *فیسبوک دات کام* می باشد"
	},
	//RENREN_ID_INSTRUCTIONS
	{
		L"Renren ID: if your profile URL is www.renren.com/12345678/profile,\nenter 12345678",
		L"人人ID：假设你的账户主页地址是\"www.renren.com/12345677/profile\",\n输入\"12345678\"",
		L"آیدی رن رن: آیدی شما شماره بعد از عبارت *رن رن.کام* می باشد"
	},
	//WAITING_FOR_RESPONSE
	{
		L"\n\n\nWaiting for directory server's response email.\n This should take 20-40 seconds.",
		L"\n\n\n等待中央服务器的回复。\n这将花20-40秒",
		L"\n\n\nدر حال انتظار برای دریافت ایمیل پاسخ از دایرکتوری سرور.\n این فرآیند ممکن است 20 الی 40 ثانیه طول بکشد"
	},







	//buttons
	//REG_FB_OR_RENREN
	{
		L"Register with Facebook",//COMMENTRENRENL"Register with Facebook or Renren",
		L"使用Facebook注册",//COMMENTRENRENL"使用Facebook或人人注册",
		L"ثبت نام با فیسبوک "
	},
	//REG_RECOMMENDED
	{
		L"Register with recommendation code",
		L"使用邀请码注册",
		L"ثبت نام با کد توصیه"
	},
	//LOGIN_BUTTON
	{
		L"Login",
		L"登陆",
		L"ورود"
	},
	//CONNECT_BUTTON
	{
		L"Connect",
		L"连接",
		L"وصل شو!"
	},
	//CANCEL_CONNECTING_BUTTON
	{
		L"Cancel connecting",
		L"取消连接",
		L"وصل نشو!"
	},
	//DISCONNECT_BUTTON
	{
		L"Disconnect",
		L"断开连接",
		L"قطع شو!"
	},
	//CANCEL_BUTTON
	{
		L"Cancel",
		L"取消",
		L"کنسل کن"
	},
	//SHOW_ADVANCED_VIEW
	{
		L"Show advanced view",
		L"显示高级视图",
		L"نمایش نمای پیشرفته "
	},
	//HIDE_ADVANCED_VIEW
	{
		L"Hide advanced view",
		L"隐藏高级视图",
		L"پنهان کردن نمای پیشرفته"
	},
	//GET_REC_CODE_FOR_FRIEND
	{
		L"Get recommendation code for friend",
		L"为你的朋友获取一个邀请码",
		L"دریافت کد توصیه برای دوستان"
	},
	//REDEEM_REC_CODE
	{
		L"Redeem recommendation code",
		L"使用邀请码",
		L"آزادکردن کد توصیه"
	},
	//WIPE_CONFIG
	{
		L"Wipe Salmon\nconfig files",
		L"清空Salmon\n所有配置文件",
		L"پاک کردن ماهی آزاد\nفایل های تنظیمات"
	},
	//SUBMIT_REGISTRATION
	{
		L"Submit Registration",
		L"提交注册",
		L"انجام ثبت نام"
	},







	//window titles
	//SELECT_LANGUAGE_TITLE
	{
		L"Select language",
		L"选择语言",
		L"انتخاب زبان"
	},
	//LOGIN_OR_REG_TITLE
	{
		L"Salmon - Login or register",
		L"Salmon - 登陆或注册",
		L"ماهی آزاد- ورود یا ثبت نام"
	},
	//SALMON_TITLE
	{
		L"Salmon",
		L"Salmon",
		L"ماهی آزاد"
	},
	//SALMON_REG_TITLE
	{
		L"Salmon - Register",
		L"Salmon - 注册",
		L"ماهی آزاد- ثبت نام"
	},
	//SALMON_PLEASE_WAIT_TITLE
	{
		L"Salmon - Please wait",
		L"Salmon - 请等待",
		L"ماهی آزاد- لطفاً منتظر بمانید"
	},


	//misc / added later
	//FACEBOOK_STR
	{
		L"Facebook",
		L"Facebook",
		L"فیسبوک"
	},
	//ERROR_STR
	{
		L"Error",
		L"错误",
		L"خطا"
	},
	//FATAL_ERROR
	{
		L"Fatal Error",
		L"致命错误",
		L"خطای مرگبار"
	},
	//DIR_SERVER_SAYS
	{
		L"Directory server says:",
		L"中央服务器信息：",
		L"دایرکتوری سرور می گوید:"
	},
	//LEGAL_WARNING
	{
L"Salmon is intended only to provide the world with the same\n\
amount of access to the internet that the U.S. has. The Salmon\n\
Project does not want you to use Salmon to violate any U.S.\n\
laws - both criminal and copyright. We will fully and cheerfully\n\
cooperate with the U.S. legal system.\n\n\
In short, if you break U.S. laws, you are not any more hidden\n\
from the U.S. government than if you weren't using Salmon.",
L"Salmon旨在提供给全世界和美国一样的网络信息。\nSalmon希望您不会将此用于任何不当途径 - 包括犯罪和版权。\nSalmon会寻求美国法律机构帮助。\nSalmon不会为您违反美国法律的行为活动提供任何庇护。\n",
L"هدف ماهی آزاد فراهم کردن دسترسی‌ به اینترنت آزاد به همان اندازه که در کشور ایالت متحده آمریکا مجاز  است، می‌باشد.\n\
.نقضِ قوانین آمریکا (قوانین جنایی یا قانون کپی‌ رایت) از اهداف پروژهٔ ماهی آزاد نیست\n\
در هر صورت اگر شما قوانین اینترنتی ایالات متحده \
را زیر پا بگذارید همانقدری قابل پیگیری هستید که بدون نرم افزار ماهی آزاد بوده اید.\n\
ما بطور کامل با سیستمِ قانونی آمریکا همکاری می‌کنیم"
	},
	//WAITING_LONGER_FOR_RESPONSE
	{
L"Still waiting for directory server's response email.\n\
If you just created the email account, this might take up to 2 more minutes. \
Or, check its inbox: it might be asking you to verify the account.\n\n\
If Salmon has successfully communicated using this email account before, the \
response is probably not coming - you should probably click Cancel.",
L"正在等待中央服务器回复。\n\n如果您使用刚新建的邮箱，可能会需要两分钟或者更多。请检查收件箱：您可能需要验证您的账户。\n\n如果Salmon已经使用过这个邮箱，可能不会有回复 - 请点击‘取消’",
L"همچنان منتظر سرور دایرکتوری هستیم\n\
اگر حساب کاربری خود را تازه وصل کرده‌اید ممکن است چند دقیقه طول بکشد.\
اینباکس ایمیلتان را چک کنید : ممکن است درخواست تایید حساب کاربری نیاز باشد.\n\n\
اگر برنامه ماهی آزاد را قبلا با این آدرس ایمیل به کار گرفته اید و اکنون جوابی نمی گیرید دکمه لغو درخواست راکلیک کنید."
	},
	//GET_CREDENTIALS_FOR_ANDROID
	{
		L"Send VPN server info to Android",
		L"向Android端发送VPN服务器信息",
		L"اطلاعات وی پی ان برای اندروید"
	},
	//IS_IT_OK_TO_MAIL_VPNS
	{
L"To connect your Android device to a VPN, Salmon will email a list of VPN IP addresses \
and passwords to your Salmon email address. You can copy+paste that info into Android's \
VPN settings. Android VPN instructions:\n\n\
1) Create new VPN profile, and choose L2TP pre-shared key (PSK) as connection type.\n\
2) Server address is the IP address sent in the email.\n\
3) Show advanced options, and set DNS server to 8.8.8.8 (Google's DNS server).\n\
4) Tap 'Save'. You will be prompted for the username+password included in the email once you try to connect.\n\n\
Is it ok for Salmon to send that email? The email will be sent to:\n\n",
L"为了与您的安卓端建立连接，Salmon会发送一些列的VPN地址和密码到您的邮箱。您可以复制粘贴到安卓端的VPN选项。安卓VPN指南：\n\n\
1)创建VPN用户资料，选择L2TP pre-shared key(PSK)为连接模式\n\
2)服务器地址选择来自邮件发送的地址\n\
3)展开‘高级选项’，将DNS服务器设置为8.8.8.8\n\
4)点击‘保存’。 当您连接时，您将被提示输入邮件中发送的账户名和密码\n\n\
是否向此邮箱发送信息：\n\n",

L"۱پروفایل وی پی ان جدیدی بسازید. با انتخاب گزینه زیر به عنوان نوع اتصال: \
L2TP\n\
۲.آدرس آی پی دریافتی در ایمیل خود را به عنوان آدرس سرور وارد کنید .\n\
۳.گزینه پیشرفته راانتخاب کنیدو آدرس دی ان اس را به آدرس دی ان اس گوگل که در زیر آمده تغییر دهید :\
8.8.8.8 \n\
۴.دکمه ذخیره را فشار دهید.زمانی که خواستید وصل شوید به یک صفحه فرستاده می شوید که  نام کاربری و گذر واژه دریافتی در ایمیل خود را وارد گنبد\n\
آیا برنامه ماهی آزاد اجازه دارد که آن ایمیل را بفرستد؟ایمیل به این آدرس فرستاده خواهد شد : \n\n"
	},
	//MAILING_TO_ANDROID
	{
		L"Mailing VPN Info to Android",
		L"向安卓端发送VPN信息",
		L"اطلاعات وی پی ان برای اندروید"
	},
	//DONE_MAILING_VPNS
	{
		L"Successfully mailed VPN login info to: ",
		L"VPN登陆信息成功发送至：",
		L"اطلاعات ورود به وی پی ان به آدرس زیر فرستاده شد:"
	},
	//INVALID_EMAIL_ADDR
	{
		L"Invalid email address! Please enter an address ending in @gmail.com, @hotmail.com, @outlook.com, or @yahoo.com.",
		L"非法邮箱地址。请输入gmail，hotmail，outlook或者yahoo邮箱",
L"لطفا آدرس ایمیل را ب شکل کامل وارد کنید: L”آدرس ایمیل را درست وارد نکرده اید \n\
@gmail.com, @hotmail.com, @outlook.com, یا @yahoo.com."
	},
	//WIPE_WARNING
	{
L"Doing this wipe will reset your salmon installation back to its state when it was installed. \
However, the central directory server will remember your account's VPN servers. If you log in \
to the Salmon program as usual with your normal account, it will automatically retrieve your \
information, and everything will be restored to before this wipe.\n\n\
If you perform the wipe, any current VPN connection will be disconnected, and the Salmon program will exit.\n\n\
Do you want to want to wipe?",
L"此举将卸载您的Salmon并重置全部设置。但是中央服务器会保留您的VPN服务器。如果您再次登录Salmon，您将自动获取此前的信息，所有项目将被恢复。\n\n\
 如果您开始清理，现在的VPN连接将被终端，并且Salmon会退出。\n\n\
 您是否想要清理？",
L" با این پاکسازی برنامه ماهی آزاد به حالتی برمی گردد ک تازه نصب شده بود.\
اما سرور دایرکتوری حساب وی پی ان شما را به خاطر می آورد.\
اگر مثل همیشه وارد برنامه ماهی آزاد شوید اطلاعات کاربریتان بازگردانده می شود.\n\n\
اگر این پاکسازی را انجام دهید، ارتباط وی پی ان قطع می شودو برنامه ماهی آزاد خارج می شود.آیا همچنان می خواهید ادامه دهید؟ "
	},
	//WIPE_COMPLETE
	{
		L"Wipe completed. Salmon will now exit. You can recover your information by logging in as usual.",
		L"清除完毕。Salmon将退出。下次登录时将恢复您的信息。",
		L"ماهی آزاد اجرایش متوقف میشود. شما می توانید اطلاعات خود را با ورود دوباره بازیابی کنید."
	},
	//WIPE_TITLE
	{
		L"Full wipe of Salmon's data",
		L"全面删除Salmon数据",
		L"پاکسازی کامل داده های برنامه ماهی آزاد"
	},
	//WRONG_RECCODE_LENGTH
	{
		L"Recommendation codes are supposed to be 7 characters - check for typos!",
		L"邀请码应为7位，请确认无输入错误",
		L"کد توصیه نامه باید هفت رقمی باشد. کد را دوباره وارد کنید!"
	},
	//GET_CREDENTIALS_FOR_IOS
	{
		L"Send VPN server info to iOS",
		L"向iOS发送VPN服务器信息",
		L"اطلاعات وی پی ان را بفرست به ios"
	},
	//IS_IT_OK_TO_MAIL_VPNS_IOS
	{
L"To connect your iOS device to a VPN, Salmon will email a list of '.mobileconfig' files, \
which contain IP addresses and passwords, to your Salmon email address. Each VPN server's \
information will come in its own .mobileconfig attachment. Simply tapping a .mobileconfig \
file should cause iOS to load the information as a new entry in its list of VPN servers, \
and you can then connect to it.\n\n\
This mail will be sent to:\n\n",
//TODO TODO this is google translate
		 L"Salmon会发送“.mobileconfig”文件列表到您的电子邮件地址。只需轻按一个“.mobileconfig”文件，添加一个VPN连接到您的手机。该邮件将被发送到：",
L" \
'برای اینکه سیستم اپل خود را به وی پی ان وصل کنید، فایلی  لازم دارید که برنامه ماهی آزاد با پسوند زیر ایمیل میکند: \n\
'.mobileconfig' \n\
همچنین در آن ایمیل آدرس آی پی و گذرواژه حساب کاربری ماهی آزاد خود را دریافت می کنید. \n\
هر سرور وی پی ان فایل مخصوص خود را دارد که پسوند فایل مشابه پسوند گفته شده در بالاست. \n\
برای وصل شدن فقط کافی است روی فایل کلیک کنید. \n\
ایمیل به آدرس زیر فرستاده می شود: \n\n"

	},
	//MAILING_TO_IOS
	{
		L"Mailing VPN Info to iOS",
		L"向iOS发送VPN信息",
		L"  فرستادن اطلاعات وی پی ان به سیستم عامل "
	},
	//MOBILECONFIG_HOWTO
	{
		L"The 'mobileconfig' files attached to this email should be readable by your iOS device. Just tap them to add them to your phone's list of VPNs.",
		L"此邮件上附属的‘mobilconfig’文件为iOS可读。请直接点击将它们加入您的手机的VPN列表",
L" فایل پیوست شده با پسوند زیر را دریافت کرده اید:\
'mobileconfig' \
با کلیک کردن روی آن فایل سیستم عامل اطلاعات را می خواند و به عنوان گزینه دیگری به لیست وی پی ان ها اضافه می کند"
	},
	//ANDROID_VPNLIST_HEADER
	{
		L"Try these servers. Servers higher in the list are more likely to be online and fast.\n*IMPORTANT NOTE*: no matter which you choose, be sure to expand 'advanced options' and set DNS server to 8.8.8.8! \n\n",
		L"请尝试列出的服务器。排位高的服务器可能会更快。*当紧*:展开‘高级选项’，将DNS服务器设置为8.8.8.8!\n\n",
L"این سرور ها را امتحان کنید. گزینه هایی که در لیست بالاترند معمولا پر سرعت ترند و به احتمال زیادتری آنلاین هستند\n\n\
توجه توجه : هر سروری که انتخاب کردید به قسمت تنظیمات پیشرفته بروید\
و آدرس دی ان اس خود را به سرور زیر تغییر دهید :\
8.8.8.8!\n\n"
	},
	//CANT_SEND_EMAIL_WHILE_CONNECTED
	{
L"The operation you just requested needs to send an email. To prevent the Salmon system \
from being used for email spam, your VPN server blocks the specific method of sending email \
that the Salmon client uses. (Of course, you can still access Gmail etc. through their web \
interfaces, by visiting e.g. mail.google.com in your web browser.)\n\n\
Please disconnect, and then try this operation again.",
L"此操作需要发送电子邮件。请断开VPN，然后重试。", //TODO TODO this is google translate
L"عملیات درخواستی شما نیاز به فرستادن ایمیل دارد\
از آنجا که ما نمیخواهیم از برنامه ماهی آزاد برای فرستادن ایمیل های اسپم استفاده شود، سرور وی پی ان شما این حات خاص ایمیل را بلاک می کند\
البته شما همچنان می توانید ایمیل های گوگل و غیره خود را در  صفحه براوزر چک کنید.\n\n\
لطفا وی پی ان را قطع کنید و این عملیات را باز تکرار کنید "
	},
	//GAVE_UP_DUE_TO_CONNECTIVITY_PROBLEMS
	{
L"Your internet connection appears to be down, or unstable. Please try connecting again when your connection has stabilized.",
L"你的互联网连接似乎已关闭，或不稳定。请尝试重新连接时，您的连接已经稳定。", //TODO TODO this is google translate
L"اینترنت شما به نظر می رسد که قطع شده است یا پایدار نیست. لطفا بعد از اینکه ابنترنت شما پایدار شد دوباره تلاش کنید وصل شوید."
	},
	//WHY_EMAIL_PASSWORD
	{
L"The Salmon client needs an email password to be able to communicate with the directory server over email. \
(We use email because the messages - first time registration, asking for a new server, asking for a recommendation \
code for a friend - are small+infrequent enough to fit in email, and blocking all major email \
providers would be a very extreme step for a censorship organization to take). \n\n\
For maximum safety, use a dummy email account created just for use with Salmon. Be sure the account's address isn't \
something that might identify you, and definitely don't use the same password as on your real email account.\n\n\n\n\
More details:\n\
The password is never sent to the Salmon Project or anywhere else, or written to your computer's hard disk: \
it is only used to send and receive emails, and only to/from the address cs-backend-salmon@mx.uillinois.edu. \
HOWEVER, it is certainly possible for a program like this to do bad things with your email. We can only promise \
that we don't. To be sure we don't, you or someone you trust should either examine the \
source code (https://github.com/SalmonProject/SalmonWindowsClient), \
or be socially close enough to Salmon Project people to feel safe trusting us.\n\n\
MOST IMPORTANTLY, even though the Salmon Project's version of this client does not do anything bad with your password, \
someone could create and distribute a look-alike version that does. Please verify that you have an original \
copy of the program by following the signature verification instructions in SignatureHelp.png. If you can, get the \
original SignatureHelp.png directly from https://salmon.cs.illinois.edu/SignatureHelp.png",

L"The Salmon client needs an email password to be able to communicate with the directory server over email. \
(We use email because the messages - first time registration, asking for a new server, asking for a recommendation \
code for a friend - are small+infrequent enough to fit in email, and blocking all major email \
providers would be a very extreme step for a censorship organization to take). \n\n\
For maximum safety, use a dummy email account created just for use with Salmon. Be sure the account's address isn't \
something that might identify you, and definitely don't use the same password as on your real email account.\n\n\n\n\
More details:\n\
The password is never sent to the Salmon Project or anywhere else, or written to your computer's hard disk: \
it is only used to send and receive emails, and only to/from the address cs-backend-salmon@mx.uillinois.edu. \
HOWEVER, it is certainly possible for a program like this to do bad things with your email. We can only promise \
that we don't. To be sure we don't, you or someone you trust should either examine the \
source code (https://github.com/SalmonProject/SalmonWindowsClient), \
or be socially close enough to Salmon Project people to feel safe trusting us.\n\n\
MOST IMPORTANTLY, even though the Salmon Project's version of this client does not do anything bad with your password, \
someone could create and distribute a look-alike version that does. Please verify that you have an original \
copy of the program by following the signature verification instructions in SignatureHelp.png. If you can, get the \
original SignatureHelp.png directly from https://salmon.cs.illinois.edu/SignatureHelp.png",

L"نرم افزار ماهی آزاد به پسورد ایمیل شما نیاز دارد تا بتواند با سرور دایرکتوری تماس بگبرد.\n\
این برنامه از ایمیل شما پیامی را به سرور دایرکتوری می فرستد و پاسخی دریافت می کند.\n\
محتوای این پبام ها موارد زیر است:\n\
ثبت نام کردن شما برای بار اول، درخواست سرور جدید، درخواست کد اعتبار برای دوستان.\n\
دلیلی که ما از ایمیل شما برای این دستورها استفاده می کنیم این است که \n\
بستن تمام وبسایت هایی که خدمات ایمیل ارایه می دهند برای سانسورچی کار خیلی افراطی است و احتمالا تا این حد جلو نمی روند. \n\
برای امنیت بیشتر، توصیه می کنیم یک ایمیل جدید بسازید و سعی کنید آدرس ایمیل جوری باشد که به هویت شما ربطی نداشته باشد\n\
و فقط از این ایمیل برای وصل شدن به نرم افزار مااستفاده کنید. همچنین از پسوردی استفاده نکنید که \n\
پسورد ایمیل اصلی تان است. با همه ی این ها، توجه کنید که\n\
پسورد شما هرگز به پروژه ماهی آزاد یا هیچ جای دیگری فرستاده نمی شود یا حتی در هارد دیسک کامپبوترنان نیز ذخیره نمی شود.\n\
تنها ما برای فرستادن پیام به یک کامپیوتر خاص در آدرس زبر از ایمیل شما استفاده می کنیم \n\
cs-backend-salmon@mx.uillinois.edu\n\
با این حال ما به شما هشدار می دهیم که از پسورد خودتان محافظت کنید. \n\
: برنامه هایی شبیه ماهی آزاد می توانند به شما آسیب جدی بزنند. کد برنامه ما در این آدرس اختیار شما است \n\
(https://github.com/SalmonProject/SalmonWindowsClient)    \n\
شما می توانید خودتان ببینید یا از کسی که با برنامه نویسی آشنا است بخواهید چک کند، که ما چگونه از اطلاعات شما استفاده می کنیم.\n\
به هر حال شما می توانید یک آدرس ایمیل در هر سایتی که خواستید مخصوص وصل شدن به برنامه ماهی آزاد درست کنید و فقط برای این منظور از آن استفاده کنبد.\n\
در این صورت سعی کنید ایمیلی که می سازید هیج ربطی به هویت شما یا ایمیل های دیگر یا پسوردهای دیگرتان نداشته باشد\n\
مهمتر از همه چیز! با اینکه پروژه ماهی آزاد به دنبال آسیب رساندن به شما نیست اما ممکن است کسان دیگری این برنامه را تغییر دهند \n\
تا از اطلاعات شما استفاده کنند. لطفا چک کنید که کپی اصلی این برنامه به دست شما رسیده است. \n\
ما این برنامه را رمز گذاری کرده ایم و شما می توانید با دنبال کردن دستور خاصی ببینید که برنامه شما اصلی است یا نه. \n\
SignatureHelp.png \n\
فایلی است که دستور لازم را به شما می دهد و همه ی ورژن های ماهی آزاد باید این فایل را داشته باشد. \n\
مسیر درست دراین فایل این است\n\
USERTrust -> USERTrust RSA Certification Authority -> InCommon RSA Server CA -> salmon.cs.illinois.edu"
	},
	//ISNT_THIS_UNSAFE_BUTTON
	{
L"(Isn't this unsafe?!)",
L"(Isn't this unsafe?!)", //NOTE: this is in a fairly narrow button... if translating, check that it's still visible.
L"درباره امنیت"
	},
	//SALMON_CANT_EMAIL_Q
	{
		L"Client failing to send emails?",
		L"Client failing to send emails?",
		L"شکست برای ارسال ایمیل؟", //TODO this is google translate
	},
	//MANUAL_EMAIL_TOGGLE_CAPTION
	{
		L"If this program says 'failed to send email', check this box to send the emails manually: the program will give you some text to copy+paste into e.g. mail.google.com.",
		L"If this program says 'failed to send email', check this box to send the emails manually: the program will give you some text to copy+paste into e.g. mail.google.com.",
		L"برای فرستادن ایمیل ها از طرف برنامه و به صورت دستی اینجا را کلیک کنید"
	},
	//ENTER_ANY_EMAIL_ADDRESS
	{
		L"Enter any email address:",
		L"Email 邮箱:",
		L"هر آدرس ایمیل را وارد کنید" //TODO UPDATE (from this google translate), should be just "any address"
	},
	//ENTER_ANY_EMAIL_ADDRESS_VERBOSE eh... the verbose stuff no longer applies when it's manual (which is what this is for)
	{
		L"Enter any email address:",
		L"Email 邮箱:",
		L"هر آدرس ایمیل را وارد کنید" //TODO UPDATE (from this google translate), should be just "any address"
	},
	//COPY_THIS_BODY
	{
		L"Copy+paste into email body:",
		L"Copy+paste into email body:",
		L"Email body:"
	},
	//COPY_THIS_SUBJECT
	{
		L"Copy+paste into email subject:",
		L"Copy+paste into email subject:",
		L"Email subject:"
	},
	//OPEN_RECEIVED_ATTACHMENT
	{
		L"Open downloaded attachment",
		L"Open downloaded attachment",
		L"گسترش فایل های دانلود شده"
	},
	//USE_THIS_BUTTON_TO_SELECT_ATTACHMENT
	{
		L"Copy the above information into an email and send it, and in ~30 seconds you should receive a reply. The reply will have an attachment. Please download the attachment, and then click this button and specify where you saved the file.",
		L"Copy the above information into an email and send it, and in ~30 seconds you should receive a reply. The reply will have an attachment. Please download the attachment, and then click this button and specify where you saved the file.",
		L"اطلاعات بالا را در یک ایمیل کپی کنید و بفرستید.تقریبا یک دقیقه بعد باید یک جواب دریافت کنید.جواب یک فایل ضمیمه دارد، لطفا آن را دانلود کنید و سپس روی این دکمه کلیک کنید و جایی که فایل را ذخیره کرده اید مشخص کنید."
	},
	//COPY_THIS_ADDRESS_TO_SEND
	{
		L"Copy+paste this address to send to:",
		L"Copy+paste this address to send to:",
		L"نشانی به:"
	},
	//FILE_DOES_NOT_EXIST
	{
		L"File does not exist or cannot be read!",
		L"File does not exist or cannot be read!",
		L"File does not exist or cannot be read!"
	}
};


const WCHAR* localizeConst(LocalizedString theStrEnumMember)
{
	return allStrings[theStrEnumMember][gChosenLanguage];
}

SalmonLanguage loadLanguageFromConfig()
{
	char fileBuf[300];
	FILE* readLangFile = openConfigFile("locale.txt", "rt");
	if (!readLangFile)
		return SALMON_LANGUAGE_INVALID;
	if (fread(fileBuf, 1, 2, readLangFile) < 2)
	{
		fclose(readLangFile);
		return SALMON_LANGUAGE_INVALID;
	}
	fclose(readLangFile);
	fileBuf[2] = 0;

	if (!strcmp(fileBuf, "CN"))
	{
		gChosenLanguage = SALMON_LANGUAGE_ZH;
		//COMMENTRENRENSendMessage(rdioRenren, BM_SETCHECK, 1, 0);
		gCurSocialNetwork = SOCNET_FACEBOOK;//COMMENTRENRENgCurSocialNetwork = SOCNET_RENREN;
		//COMMENTRENRENStatic_SetText(sttcSocNetID, localizeConst(RENREN_ID_INSTRUCTIONS));
	}
	else if (!strcmp(fileBuf, "IR"))
	{
		//COMMENTRENRENSendMessage(rdioFacebook, BM_SETCHECK, 1, 0);
		gCurSocialNetwork = SOCNET_FACEBOOK;
		gChosenLanguage = SALMON_LANGUAGE_IR;
		//sttcSocNetID should default to facebook, so don't bother setting it
	}
	else
	{
		//COMMENTRENRENSendMessage(rdioFacebook, BM_SETCHECK, 1, 0);
		gCurSocialNetwork = SOCNET_FACEBOOK;
		gChosenLanguage = SALMON_LANGUAGE_EN;
		//sttcSocNetID should default to facebook, so don't bother setting it
	}
	setAllText();
	return gChosenLanguage;
}

void saveLanguageToConfig(SalmonLanguage theLang)
{
	FILE* writeLangFile = openConfigFile("locale.txt", "wt");
	if (!writeLangFile)
		return;
	if (theLang == SALMON_LANGUAGE_ZH)
		fwrite("CN", 1, 2, writeLangFile);
	else if (theLang == SALMON_LANGUAGE_IR)
		fwrite("IR", 1, 2, writeLangFile);
	else
		fwrite("US", 1, 2, writeLangFile);

	fclose(writeLangFile);
}

//Returns an upper-case country code TLD corresponding to the chosen language.
//(Currently: Chinese = PRoC, Persian = Iran, default = USA)
const WCHAR* countryFromLanguage(SalmonLanguage theLang)
{
	switch (theLang)
	{
	case SALMON_LANGUAGE_ZH:
		return L"CN";
	case SALMON_LANGUAGE_IR:
		return L"IR";
	default:
		return L"US";
	}
}


void setAllText()
{
	SetWindowText(wndwLogin,				localizeConst(LOGIN_OR_REG_TITLE));
	SetWindowText(wndwMain ,				localizeConst(SALMON_TITLE));
	SetWindowText(wndwRegisterRecd,			localizeConst(SALMON_REG_TITLE));
	SetWindowText(wndwRegisterSocial,		localizeConst(SALMON_REG_TITLE));
	SetWindowText(wndwWaiting,				localizeConst(SALMON_PLEASE_WAIT_TITLE));
	SetWindowText(sttcWaiting,				localizeConst(WAITING_FOR_RESPONSE));
	SetWindowText(bttnCancelWaiting,		localizeConst(CANCEL_BUTTON));
	SetWindowText(sttcRegRecCode,			localizeConst(REC_CODE_FROM_FRIEND));
	SetWindowText(sttcRecEmailAddr,			localizeConst(PROMPT_EMAIL_ADDR_VERBOSE));
	SetWindowText(sttcRecEmailPW,			localizeConst(PROMPT_EMAIL_PW_VERBOSE));
	SetWindowText(bttnIsntThisUnsafeMain,	localizeConst(ISNT_THIS_UNSAFE_BUTTON));
	SetWindowText(bttnRecRegSubmit,			localizeConst(SUBMIT_REGISTRATION));
	SetWindowText(textEnterPost,			localizeConst(ENTER_POST_INSTRUCTIONS));
	//COMMENTRENRENSetWindowText(rdioFacebook,			localizeConst(FACEBOOK_STR));
	//COMMENTRENRENSetWindowText(rdioRenren,			localizeConst(RENREN_STR));
	SetWindowText(sttcSocNetID,				localizeConst(FACEBOOK_ID_INSTRUCTIONS));
	SetWindowText(sttcSocEmailAddr,			localizeConst(PROMPT_EMAIL_ADDR_VERBOSE));
	SetWindowText(sttcSocEmailPW,			localizeConst(PROMPT_EMAIL_PW_VERBOSE));
	SetWindowText(bttnIsntThisUnsafeSocReg, localizeConst(ISNT_THIS_UNSAFE_BUTTON));
	SetWindowText(bttnSocRegSubmit,			localizeConst(SUBMIT_REGISTRATION));
	SetWindowText(sttcConnectStatus,		gVPNConnected ? localizeConst(VPN_STATUS_CONNECTED) : localizeConst(VPN_STATUS_DISCONNECTED));
	SetWindowText(bttnConnect,				localizeConst(CONNECT_BUTTON));
	SetWindowText(bttnCancelConnect,		localizeConst(CANCEL_CONNECTING_BUTTON));
	SetWindowText(bttnDisconnect,			localizeConst(DISCONNECT_BUTTON));
	SetWindowText(bttnShowHideOptions,		localizeConst(SHOW_ADVANCED_VIEW));
	SetWindowText(sttcTrustStatus,			localizeConst(TRUST_LVL_ORDINARY));
	SetWindowText(bttnGetRec,				localizeConst(GET_REC_CODE_FOR_FRIEND));
	SetWindowText(framRedeemCode,			localizeConst(BOOST_TRUST_W_CODE));
	SetWindowText(bttnRedeemCode,			localizeConst(REDEEM_REC_CODE));
	SetWindowText(bttnWipeConfig,			localizeConst(WIPE_CONFIG));
	SetWindowText(framFirstTime,			localizeConst(FIRST_TIME_USER_Q));
	SetWindowText(bttnRegisterSocial,		localizeConst(REG_FB_OR_RENREN));
	SetWindowText(bttnRegisterRecd,			localizeConst(REG_RECOMMENDED));
	SetWindowText(framExistingUser,			localizeConst(EXISTING_USER_Q));
	SetWindowText(sttcLoginEmailAddr,		localizeConst(PROMPT_EMAIL_ADDR));
	SetWindowText(sttcLoginEmailPW,			localizeConst(PROMPT_EMAIL_PASSWORD));
	SetWindowText(bttnIsntThisUnsafeRecReg, localizeConst(ISNT_THIS_UNSAFE_BUTTON));
	SetWindowText(bttnLogin,				localizeConst(LOGIN_BUTTON));
	SetWindowText(bttnAndroidGetter,		localizeConst(GET_CREDENTIALS_FOR_ANDROID));
	SetWindowText(bttniOSGetter,			localizeConst(GET_CREDENTIALS_FOR_IOS));
	SetWindowText(framCantEmailFrame,		localizeConst(SALMON_CANT_EMAIL_Q));
	SetWindowText(chekManualEmail,			localizeConst(MANUAL_EMAIL_TOGGLE_CAPTION));
	SetWindowText(sttcManualEmailTo,		localizeConst(COPY_THIS_ADDRESS_TO_SEND));
	SetWindowText(sttcManualEmailSubject,	localizeConst(COPY_THIS_SUBJECT));
	SetWindowText(sttcManualEmailBody,		localizeConst(COPY_THIS_BODY));
	SetWindowText(sttcManualFindFileDescription,localizeConst(USE_THIS_BUTTON_TO_SELECT_ATTACHMENT));
	SetWindowText(bttnManualEmailFindRecvd,	localizeConst(OPEN_RECEIVED_ATTACHMENT));
}
