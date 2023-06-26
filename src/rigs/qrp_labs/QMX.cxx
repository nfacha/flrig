// ----------------------------------------------------------------------------
// Copyright (C) 2014
//              David Freese, W1HKJ
//
// This file is part of flrig.
//
// flrig is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// flrig is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include "qrp_labs/QMX.h"
#include "support.h"

static const char QMXname_[] = "QMX";

static std::vector<std::string>vQMXmodes_;
static const char *QMXmodes_[] = {
	"CW", "CW-R", "LSB", "USB"};
static const char QMX_mode_type[] = { 'L', 'U' };
static const char *QMX_mode_str[] = {
	"MD3;", "MD7;", "MD1;", "MD2;", NULL };

static std::vector<std::string>vQMX_BW;
static const char *QMX_BW[] = {
  "3200" };

static GUI rig_widgets[]= {
	{ (Fl_Widget *)NULL,          0,   0,   0 }
};

void RIG_QMX::initialize()
{
	VECTOR (vQMXmodes_, QMXmodes_);
	VECTOR (vQMX_BW, QMX_BW);

	modes_ = vQMXmodes_;
	bandwidths_ = vQMX_BW;
}

RIG_QMX::RIG_QMX() {

	name_ = QMXname_;
	modes_ = vQMXmodes_;
	bandwidths_ = vQMX_BW;

	widgets = rig_widgets;

	serial_baudrate = BR9600;
	stopbits = 1;
	serial_retries = 2;
	serial_rtscts  = false;
	serial_rtsplus = false;
	serial_dtrplus = false;
	serial_rtsptt  = false;
	serial_dtrptt  = false;

	serial_write_delay = 0;
	serial_post_write_delay = 0;
	serial_timeout = 50;

	serial_catptt  = true;

	B.imode = A.imode = 1;
	B.iBW = A.iBW = 0;
	A.freq = 7070000ULL;
	B.freq = 14070000ULL;

	has_mode_control =
	has_bandwidth_control =
	has_extras =
	has_volume_control =
	has_ptt_control =
	has_vfo_adj =
	has_vox_onoff =
	has_rit =
	has_split =
	can_change_alt_vfo = true;

	precision = 1;
	ndigits = 8;

}

static int ret = 0;
static int split_ = 0;
static int rit_ = 0;
static int rit_stat_ = 0;

static char cmdstr[20];

inline void TRACE_CMD(std::string func, std::string cmd )
{
//	std::cout << ztime() << "::" << func << " sent: " << cmd << std::endl;
}

inline void TRACE_REP(std::string func, std::string replystr)
{
//	std::cout << ztime() << "::" << func << " rcvd: " << replystr << std::endl;
}

void RIG_QMX::shutdown()
{
}

bool RIG_QMX::check()
{
	cmd = "VN;";  // version number query; re VN1_05;
	get_trace(1, "get version");

	TRACE_CMD("check", cmd);

	wait_char(';', 8, 100, "get version", ASC);
	gett("");

//	if (replystr.find("VN") == std::string::npos) return 0;

	TRACE_REP("check", replystr);

	return 1;
}

unsigned long long RIG_QMX::get_vfoA ()
{
	cmd = "FA;";
	get_trace(1, "get vfoA");

	TRACE_CMD("get_vfoA", cmd);

	if (wait_char(';', 14, 100, "get vfo A", ASC) < 14) {

		TRACE_REP("get_vfoA", replystr);

		return A.freq;
	}
	gett("");

	size_t p = replystr.rfind("FA");

	if (p != std::string::npos) {
		sscanf(&replystr[p+2], "%llu", &A.freq);
	}

	TRACE_REP("get_vfoA", replystr);

	return A.freq;
}

void RIG_QMX::set_vfoA (unsigned long long freq)
{
	A.freq = freq;
	snprintf(cmdstr, sizeof(cmdstr), "FA%011llu;", A.freq);
	cmd = cmdstr;
	set_trace(1, "set vfoA");

	TRACE_CMD("set_vfoA", cmd);

	sendCommand(cmd);
	sett("");

	TRACE_REP("set_vfoA", replystr);

	showresp(WARN, ASC, "set vfo A", cmd, "");
}

unsigned long long RIG_QMX::get_vfoB ()
{
	cmd = "FB;";
	get_trace(1, "get vfoB");

	TRACE_CMD("get_vfoB", cmd);

	if (wait_char(';', 14, 100, "get vfo B", ASC) < 14) {

		TRACE_REP("get_vfoB", replystr);

		return B.freq;
	}
	gett("");

	size_t p = replystr.rfind("FB");

	if (p != std::string::npos) {
		sscanf(&replystr[p+2], "%llu", &B.freq);
	}

	TRACE_REP("get_vfoB", replystr);

	return B.freq;
}

void RIG_QMX::set_vfoB (unsigned long long freq)
{
	B.freq = freq;
	snprintf(cmdstr, sizeof(cmdstr), "FB%011llu;", B.freq);
	cmd = cmdstr;
	set_trace(1, "set vfoB");

	TRACE_CMD("set_vfoB", cmd);

	sendCommand(cmd);

	sett("");

	TRACE_REP("set_vfoB", replystr);

	showresp(WARN, ASC, "set vfo B", cmd, "");
}

void RIG_QMX::selectA()
{
	cmd = "FR0;FT0;";
	set_trace(1, "select A");

	TRACE_CMD("selectA", cmd);

	sendCommand(cmd);
	sett("");

	showresp(WARN, ASC, "Rx on A, Tx on A", cmd, "");
	inuse = onA;

	TRACE_REP("selectA", replystr);

}

void RIG_QMX::selectB()
{
	cmd = "FR1;FT1;";
	set_trace(1, "select B");

	TRACE_CMD("selectB", cmd);

	sendCommand(cmd);
	sett("");

	showresp(WARN, ASC, "Rx on B, Tx on B", cmd, "");
	inuse = onB;

	TRACE_REP("selectB", replystr);

}

void RIG_QMX::set_split(bool val) 
{
	if (val) cmd = "SP1;";
	else     cmd = "SP0;";
	set_trace(1, "set split on/off");

	TRACE_CMD("set_split", cmd);

	sendCommand(cmd);
	sett("");

	showresp(WARN, ASC, "set split", cmd, "");
	split_ = val;

	TRACE_REP("set_split", replystr);

}

bool RIG_QMX::can_split()
{
	return true;
}

int RIG_QMX::get_split()
{
	cmd = "SP;";
	get_trace(1, "get split");

	TRACE_CMD("get_split", cmd);

	ret = wait_char(';', 4, 100, "get split", ASC);
	gett("");

	split_ = replystr[2] == '1';

	TRACE_REP("get_split", replystr);

	return split_;
}

void RIG_QMX::set_bwA(int val)
{
	A.iBW = 0;
}

int RIG_QMX::get_bwA()
{
	return A.iBW = 0;
}

void RIG_QMX::set_bwB(int val)
{
	B.iBW = 0;
}

int RIG_QMX::get_bwB()
{
	return B.iBW = 0;
}

// Tranceiver PTT on/off
void RIG_QMX::set_PTT_control(int val)
{
	if (val) cmd = "TQ1;";
	else     cmd = "TQ0;";
	set_trace(1, "set PTT");

	TRACE_CMD("set_PTT", cmd);

	sendCommand(cmd);
	sett("");

	showresp(WARN, ASC, "set PTT", cmd, "");
	ptt_ = val;

	TRACE_REP("set_PTT", replystr);

}

int RIG_QMX::get_PTT()
{
	cmd = "TQ;";
	get_trace(1, "get PTT");

	TRACE_CMD("get_PTT", cmd);

	ret = wait_char(';', 4, 100, "get PTT", ASC);
	gett("");

	TRACE_REP("get_PTT", replystr);

	if (ret < 4) return ptt_;
	return ptt_ = replystr[2] == '1';
}

/*======================================================================

IF; response

IFaaaaaaaaaaaXXXXXbbbbbcdXeefghjklmmX;
12345678901234567890123456789012345678
01234567890123456789012345678901234567 byte #
          1         2         3
                            ^ position 28
where:
	aaaaaaaaaaa => 11 digit decimal value of vfo frequency
	XXXXX => 5 spaces
	bbbbb => 5 digit RIT frequency, -0200 ... +0200
	c => 0 = rit OFF, 1 = rit ON
	d => xit off/on; always ZERO
	X    always ZERO
	ee => memory channel; always 00
	f => tx/rx; 0 = RX, 1 = TX
	g => mode; always 3 (CW)
	h => function; receive VFO; 0 = A, 1 = B
	j => scan off/on; always ZERO
	k => split off /on; 0 = Simplex, 1 = Split
	l => tone off /on; always ZERO
	m => tone number; always ZERO
	X => unused characters; always a SPACE
======================================================================*/ 

int RIG_QMX::get_IF()
{
	cmd = "IF;";
	get_trace(1, "get_PTT");

	TRACE_CMD("get_IF", cmd);

	ret = wait_char(';', 38, 100, "get VFO", ASC);
	gett("");

	if (ret < 38) return ptt_;

	rit_ = 0;
	for (int n = 22; n > 18; n--)
		rit_ = (10 * rit_) + (replystr[n] - '0');
	if (replystr[18] == '-') rit_ *= -1;
	rit_stat_ = (replystr[23] - '0');

	ptt_ = (replystr[28] == '1');

	split_ = (replystr[32] == '1');

	TRACE_REP("get_IF", replystr);

	return ptt_;
}

void RIG_QMX::set_modeA(int val)
{
	if (val < 0 || val > 3) return;
	cmd = QMX_mode_str[val];
	set_trace(1, "set mode A");

	TRACE_CMD("set_modeA", cmd);

	sendCommand(cmd);
	sett("");
	showresp(WARN, ASC, "set mode A", cmd, "");
	A.imode = val;

	TRACE_REP("set_modeA", replystr);

}

int RIG_QMX::get_modeA()
{
	cmd = "MD;";
	get_trace(1, "get_modeA");

	TRACE_CMD("get_modeA", cmd);

	ret = wait_char(';', 4, 100, "get modeA", ASC);
	gett("");

	TRACE_REP("get_modeA", replystr);

	if (ret < 4) return A.imode;
	for (int i = 0; i < 4; i++)
		if (replystr.find(QMX_mode_str[i]) != std::string::npos)
			return A.imode = i;
	return A.imode;
}

void RIG_QMX::set_modeB(int val)
{
	if (val < 0 || val > 3) return;
	cmd = QMX_mode_str[val];
	set_trace(1, "set mode B");

	TRACE_CMD("set_modeB", cmd);

	sendCommand(cmd);
	sett("");
	showresp(WARN, ASC, "set mode B", cmd, "");
	B.imode = val;

	TRACE_REP("set_modeB", replystr);
}

int RIG_QMX::get_modeB()
{
	cmd = "MD;";
	get_trace(1, "get_modeA");

	TRACE_CMD("get_modeB", cmd);

	ret = wait_char(';', 4, 100, "get modeB", ASC);
	gett("");

	TRACE_REP("get_modeB", replystr);

	if (ret < 4) return B.imode;
	for (int i = 0; i < 4; i++)
		if (replystr.find(QMX_mode_str[i]) != std::string::npos)
			return B.imode = i;
	return B.imode;
}

void RIG_QMX::setVfoAdj(double v)
{
	if (v > 1000) v = 1000;
	if (v < -1000) v = -1000;
	vfo_ = v;//25000000 + (int)v;
	char cmdstr[12];
	snprintf(cmdstr, sizeof(cmdstr), "Q0%08.0f;", vfo_ + 25000000);
	cmd = cmdstr;
	set_trace(1, "set TCXO ref freq");

	TRACE_CMD("setVfoAdj", cmd);

	sendCommand(cmd);
	sett("");

	TRACE_REP("setVfoAdj", replystr);

}

double RIG_QMX::getVfoAdj()
{
	cmd = "Q0;";
	get_trace(1, "get TCXO ref freq");

	TRACE_CMD("getVfoAdj", cmd);

	ret = wait_char(';', 12, 100, "get TCXO ref freq", ASC);

	TRACE_REP("getVfoAdj", replystr);

	if (ret < 11) return vfo_;
	int vfo;
	sscanf( (&replystr[2]), "%d", &vfo);
	return vfo_ = vfo - 25000000;
}

void RIG_QMX::get_vfoadj_min_max_step(double &min, double &max, double &step)
{
	min = -1000; 
	max = 1000; 
	step = 1;
}

void RIG_QMX::set_vox_onoff()
{
	if (progStatus.vox_onoff) cmd = "Q31;";
	else                      cmd = "Q30;";
	set_trace(1, "set vox on/off");

	TRACE_CMD("set_vox_onoff", cmd);

	sendCommand(cmd);
	sett("");

	TRACE_REP("set_vox_onoff", replystr);

	showresp(WARN, ASC, "SET vox gain", cmd, replystr);
}

int RIG_QMX::get_vox_onoff()
{
	cmd = "Q3;";
	get_trace(1, "get vox onoff");

	TRACE_CMD("get_vox_onoff", cmd);

	ret = wait_char(';', 4, 100, "get vox on/off", ASC);
	gett("");

	TRACE_REP("get_vox_onoff", replystr);

	if (ret < 4) return progStatus.vox_onoff;
	progStatus.vox_onoff = (replystr[2] == '1');
	return progStatus.vox_onoff;
}

void RIG_QMX::set_volume_control(int val)
{
	cmd = "AG";
	char szval[20];
	snprintf(szval, sizeof(szval), "%02d", val);
	cmd += szval;
	cmd += ';';
	set_trace(1, "set vol control");

	TRACE_CMD("set_vol", cmd);

	sendCommand(cmd);
	sett("");

	TRACE_REP("set_vol", replystr);

}

int RIG_QMX::get_volume_control()
{
	int val = progStatus.volume;
	cmd = "AG;";
	get_trace(1, "get vol control");

	TRACE_CMD("get_vol", cmd);

	ret = wait_char(';', 5, 100, "get vol", ASC);
	gett("");

	TRACE_REP("get_vol", replystr);

	size_t p = replystr.rfind("AG");
	if (p == std::string::npos) return val;
	val = atoi(&replystr[p + 2]);
	return val;
}

void RIG_QMX::get_vol_min_max_step(int &min, int &max, int &step)
{
	min = 0;
	max = 99;
	step = 1;
}

void RIG_QMX::setRit(int val)
{
	if (val >= 0) snprintf(cmdstr, sizeof(cmdstr), "RU%u;", val);
	else if (val < 0) snprintf(cmdstr, sizeof(cmdstr), "RD-%u;", abs(val));
	rit_ = val;
	cmd = cmdstr;
	set_trace(1, "set RIT");

	TRACE_CMD("setRit", cmd);

	sendCommand(cmd);
	sett("");

	TRACE_REP("setRit", replystr);

}

int RIG_QMX::getRit()
{
	get_IF();
	return rit_;
}
