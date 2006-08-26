/*
 * 	printer.cpp		(C) 2006, Aurélien Croc (AP²C)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the
 *  Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *  $Id$
 *
 */
#include "printer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "error.h"

/*
 * Fonctions internes
 * Internal functions
 */
long double Printer::_convert(long double d) const
{
	return d * _resolution / 72.;
}

char *Printer::_convertStr(const char *str) const
{
	unsigned int i;
	char *out = new char[strlen(str)];

	for (i=0; *str; str++) {
		if (*str == '<' && strlen(str) >= 3 && isxdigit(*(str+1)))  {
			char temp[3] = {0, 0, 0};
			
			str++;
			temp[0] = *str;
			str++;
			if (*str != '>' && (!isxdigit(*str) || 
					*(str + 1) != '>')) {
				out[i] = '<';
				out[i+1] = temp[0];
				i += 2;
				continue;
			}
			if (*str != '>') {
				temp[1] = *str;
				str++;
			}
			out[i] = (char)strtol((char *)&temp, (char **)NULL, 16);
			i++;
			continue;
		}
		out[i] = *str;
		i++;
	}
	out[i] = 0;
	return out;
}



/*
 * Constructeur - Destructeur
 * Init - Uninit
 */
Printer::Printer(ppd_file_t *ppd)
{
	ppd_choice_t *choice;
	ppd_attr_t *attr;


	_ppd = ppd;
	_pageSizeX = 595.;
	_pageSizeY = 842.;
	_marginX = 12.5;
	_marginY = 12.5;
	_areaX = 582.5;
	_areaY = 829.5;
	_docHeaderValues = NULL;

	// Get the resolution
	if ((choice = ppdFindMarkedChoice(_ppd, "Resolution")))
		_resolution = strtol(choice->choice, (char **)NULL, 10);
	else
		_resolution = 600;

	// Get the paper type
	if ((choice = ppdFindMarkedChoice(_ppd, "MediaSize"))) {
		if (!(strcmp(choice->choice, "Letter")))
			_paperType = 0;
		else if (!(strcmp(choice->choice, "Legal")))
			_paperType = 1;
		else if (!(strcmp(choice->choice, "A4")))
			_paperType = 2;
		else if (!(strcmp(choice->choice, "Executive")))
			_paperType = 3;
		else if (!(strcmp(choice->choice, "Ledger")))
			_paperType = 4;
		else if (!(strcmp(choice->choice, "A3")))
			_paperType = 5;
		else if (!(strcmp(choice->choice, "Env10")))
			_paperType = 6;
		else if (!(strcmp(choice->choice, "Monarch")))
			_paperType = 7;
		else if (!(strcmp(choice->choice, "C5")))
			_paperType = 8;
		else if (!(strcmp(choice->choice, "DL")))
			_paperType = 9;
		else if (!(strcmp(choice->choice, "B4")))
			_paperType = 10;
		else if (!(strcmp(choice->choice, "B5")))
			_paperType = 11;
		else if (!(strcmp(choice->choice, "EnvISOB5")))
			_paperType = 12;
		else if (!(strcmp(choice->choice, "A5")))
			_paperType = 16;
		else if (!(strcmp(choice->choice, "A6")))
			_paperType = 17;
		else if (!(strcmp(choice->choice, "EnvISOB6")))
			_paperType = 18;
		else if (!(strcmp(choice->choice, "C6")))
			_paperType = 23;
		else if (!(strcmp(choice->choice, "Folio")))
			_paperType = 24;
		else {
			ERROR(_("Printer::Printer: Invalid MediaSize %s"), 
				choice->choice);
			_paperType = 2;
		}
	} else
		_paperType = 2;

	// Get the paper source
	if ((choice = ppdFindMarkedChoice(_ppd, "InputSlot"))) {
		if (!(strcmp(choice->choice, "Auto")))
			_paperSource = 1;
		else if (!(strcmp(choice->choice, "Manual")))
			_paperSource = 2;
		else if (!(strcmp(choice->choice, "Multi")))
			_paperSource = 3;
		else if (!(strcmp(choice->choice, "Upper")))
			_paperSource = 4;
		else if (!(strcmp(choice->choice, "Lower")))
			_paperSource = 5;
		else if (!(strcmp(choice->choice, "Envelope")))
			_paperSource = 6;
		else if (!(strcmp(choice->choice, "Tray3")))
			_paperSource = 7;
		else {
			ERROR(_("Printer::Printer: Invalid InputSlot %s"), 
				choice->choice);
			_paperSource = 1;
		}
	} else
		_paperSource = 1;

	// Get the duplex --- TODO 
	/** @todo */
	_duplex = 0x0100;

	// Get the compression version
	attr = ppdFindAttr(_ppd, "General", "compVersion");
	if (attr)
		_compVersion = strtol(attr->value, (char **)NULL, 16);
	else
		_compVersion = 0x11;

	// Get the doc header values
	attr = ppdFindAttr(_ppd, "General", "docHeaderValues");
	if (attr)
		_docHeaderValues = _convertStr(attr->value);
	else {
		_docHeaderValues = new char[5];
		_docHeaderValues[0] = 0;
		_docHeaderValues[1] = 0;
		_docHeaderValues[2] = 1;
		_docHeaderValues[3] = 0;
		_docHeaderValues[4] = 0;
	}

}

Printer::Printer()
{
	_resolution = 600;
	_paperType = 2;
	_paperSource = 1;
	_duplex = 0x0100;
	
	_pageSizeX = 595.;
	_pageSizeY = 842.;
	_marginX = 12.5;
	_marginY = 12.5;
	_areaX = 582.5;
	_areaY = 829.5;

	_docHeaderValues = new char[5];
	_docHeaderValues[0] = 0;
	_docHeaderValues[1] = 0;
	_docHeaderValues[2] = 1;
	_docHeaderValues[3] = 0;
	_docHeaderValues[4] = 0;
}

Printer::~Printer()
{
	delete[] _docHeaderValues;
}


/*
 * Émission PJL
 * PJL header and footer
 */
void Printer::newJob(FILE *output)
{
	ppd_choice_t *choice;
	ppd_attr_t *attr;

	// Send the PJL Begin
	attr = ppdFindAttr(_ppd, "PJL", "BeginPJL");
	if (attr) {
		char *tmp;
		tmp = _convertStr(attr->value);
		fprintf(output, "%s", tmp);
		delete[] tmp;
	} else
		fprintf(output, "%%-12345X");
	
	// Get the paper type
	if ((choice = ppdFindMarkedChoice(_ppd, "MediaType"))) {
		if (!strcmp(choice->choice, "OFF"))
			fprintf(output, "@PJL SET PAPERTYPE = OFF\n");
		else
			fprintf(output, "@PJL SET PAPERTYPE = %s\n", 
				choice->choice);
	} else
		fprintf(output, "@PJL SET PAPERTYPE = OFF\n");

	// Get the toner density
	if ((choice = ppdFindMarkedChoice(_ppd, "TonerDensity")))
		fprintf(output, "@PJL SET DENSITY = %s\n", choice->choice);
	else
		fprintf(output, "@PJL SET DENSITY = 3\n");

	// Get the economode state
	if ((choice = ppdFindMarkedChoice(_ppd, "EconoMode")) && 
		strcmp(choice->choice, "0")) {
		if (!strcmp(choice->choice, "ON")) {
			fprintf(output, "@PJL SET ECONOMODE = ON\n");
		} else
			fprintf(output, "@PJL SET ECONOMODE = OFF\n");
	}

	// Get the powersave state
	if ((choice = ppdFindMarkedChoice(_ppd, "PowerSave")) && 
			strcmp(choice->choice, "False")) {
		fprintf(output, "@PJL SET POWERSAVE = ON\n");
		fprintf(output, "@PJL SET POWERSAVETIME = %s\n", 
			choice->choice);
	} else {
		fprintf(output, "@PJL SET POWERSAVE = OFF\n");
	}

	// Get the jam recovery state
	if ((choice = ppdFindMarkedChoice(_ppd, "JamRecovery")) && 
			!strcmp(choice->choice, "True"))
		fprintf(output, "@PJL SET JAMERECOVERY = ON\n");
	else
		fprintf(output, "@PJL SET JAMERECOVERY = OFF\n");

	// Get the SRT mode
	if ((choice = ppdFindMarkedChoice(_ppd, "SRTMode"))) {
		fprintf(output, "@PJL SET RET = %s\n", choice->choice);
	} else
		fprintf(output, "@PJL SET RET = NORMAL\n");
}

void Printer::endJob(FILE *output)
{
	ppd_attr_t *attr;

	attr = ppdFindAttr(_ppd, "PJL", "EndPJL");
	if (attr) {
		char *tmp;

		tmp = _convertStr(attr->value);
		fprintf(output, "%s", tmp);
		delete[] tmp;
	} else
		fprintf(output, "\t%%-12345X");
}


/*
 * Calcul des dimensions à la résolution demandée
 * Calculate dimensions for the approprioate resolution
 */
