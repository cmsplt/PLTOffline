#!/cvmfs/cms-bril.cern.ch/brilconda3/bin/python3

import dataclasses
import getpass
import io
import json
import logging
import os
import pathlib
import shlex
import socket
import subprocess
import sys
import time
import typing
import urllib3

import dateutil
import requests
import pandas

logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.DEBUG)
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

if not 'lxplus' in socket.gethostname():
    sys.exit(logging.error('please run this script from lxplus'))

# cookie_file: pathlib.Path = pathlib.Path(f'cmsoms.cookie')
cookie_file: pathlib.Path = pathlib.Path(f'{pathlib.Path.home()}/private/cmsoms.cookie')
endpoint: str = 'https://cmsoms.cern.ch'

user: str = getpass.getuser()
path: pathlib.Path = pathlib.Path(f'/eos/user/{user[0]}/{user}/www/fills/')
path.mkdir(exist_ok=True)

def demo():
    # fills = pandas.read_csv('https://delannoy.web.cern.ch/fills.csv', index_col=[0], parse_dates=['oms_start_time','oms_start_stable_beam','oms_end_stable_beam','oms_end_time'], date_parser=lambda x: pandas.to_datetime(x, format='%Y-%m-%dT%H:%M:%SZ', utc=True))
    url = 'https://delannoy.web.cern.ch/fills.csv'
    parseDates = ['oms_start_time','oms_start_stable_beam','oms_end_stable_beam','oms_end_time']
    dateParser = lambda x: pandas.to_datetime(x, format='%Y-%m-%dT%H:%M:%SZ', utc=True)
    fills = pandas.read_csv(url, index_col=[0], parse_dates=parseDates, date_parser=dateParser)
    print(fills.info())
    print(fills[fills.oms_stable_beams == True].iloc[-1])


@dataclasses.dataclass
class SSO:
    '''https://gitlab.cern.ch/authzsvc/tools/auth-get-sso-cookie/'''
    capture_output: bool = False
    cert_verify: bool = False
    debug: bool = False
    stderr: int = subprocess.DEVNULL
    verbose: bool = False

    def cmd(self) -> str:
        cmd = f'auth-get-sso-cookie --url {endpoint} --outfile {cookie_file}'
        if not self.cert_verify:
            cmd += ' --nocertverify'
        if self.verbose:
            cmd += ' --verbose'
        if self.debug:
            cmd += ' --debug'
        if self.debug or self.verbose:
            self.capture_output = True
            self.stderr = None
        return cmd

    def auth(self) -> int:
        cmd = self.cmd()
        sso = subprocess.run(cmd.split(), stderr=self.stderr, capture_output=self.capture_output, text=True)
        logging.debug(sso.args)
        if sso.stdout:
            logging.debug(sso.stdout)
        if sso.stderr:
            logging.error(sso.stderr)
        return sso.returncode

    def cernSSO(self):
        # source /cvmfs/sft.cern.ch/lcg/nightlies/dev4/Fri/Python/3.9.12/x86_64-centos7-gcc11-opt/Python-env.sh
        from auth_get_sso_cookie import cern_sso
        cern_sso.save_sso_cookie(url='https://cmsoms.cern.ch', file='cmsoms.cookie', verify_cert=False, auth_hostname='auth.cern.ch')
        # import requests
        # from bs4 import BeautifulSoup # import lxml.html
        # AUTH_HOSTNAME = 'auth.cern.ch'
        # AUTH_REALM = 'cern'
        # session = requests.Session()
        # login_page = session.get(url=endpoint, verify=self.cert_verify)
        # soup = BeautifulSoup(login_page.text, features='html.parser') # tree = lxml.html.fromstring(login_page.text)
        # kerberos_button = soup.find(id='social-kerberos')
        # kerberos_path = kerberos_button.get('href') # kerberos_path = tree.xpath('//a[@id="social-kerberos"]')[0].attrib.get('href')
        # kerberos_redirect = session.get(url=f'https://{AUTH_HOSTNAME}{kerberos_path}')
        # # import requests_gssapi # https://github.com/pythongssapi/requests-gssapi
        # # import gssapi # [https://github.com/pythongssapi/python-gssapi]
        # # kerberos_auth = session.get(url=kerberos_redirect.url, auth=requests_gssapi.HTTPSPNEGOAuth(mutual_authentication=requests_gssapi.OPTIONAL), allow_redirects=False)
        # # while kerberos_auth.status_code == 302 and AUTH_HOSTNAME in kerberos_auth.headers.get('Location'):
        # #     kerberos_auth = session.get(url=kerberos_auth.headers.get('Location'), allow_redirects=False)


@dataclasses.dataclass
class BRILCALC:
    '''https://cmslumi.web.cern.ch/'''
    stable_beams: bool = None
    unit: str = None
    year: int = None
    begin: str = None
    end: str = None

    def __post_init__(self):
        self.csvFile = pathlib.Path(f'{path}/{self.year}_{self.__class__.__name__}.csv')

    @staticmethod
    def parseDatetime(dt:str) -> str:
        '''Parse datetime string into the incredibly-specific format expected by `brilcalc`.'''
        try:
            return dateutil.parser.parse(dt).strftime('%m/%d/%y %H:%M:%S')
        except dateutil.parser._parser.ParserError:
            return dt

    @staticmethod
    def aggPerFill(data:pandas.DataFrame) -> pandas.DataFrame:
        '''Aggregate per-ls results into per-fill.'''
        unit = data.columns.str.extract('^delivered\((.*)\)').dropna().squeeze()
        aggDict = {'datetime':'min', 'time':'min', 'nls':list, 'ncms':list, 'run':list, f'delivered({unit})':'sum', f'recorded({unit})':'sum'}
        aggDict = {k:v for k,v in aggDict.items() if k in data.columns.to_list()}
        return data.groupby('fill').agg(aggDict).reset_index()

    def postProcess(self, data: pandas.DataFrame) -> pandas.DataFrame:
        data = pandas.concat([data['run:fill'].str.split(':', expand=True).rename(columns={0:'run', 1:'fill'}), data], axis=1)
        data = data.apply(pandas.to_numeric, errors='ignore')
        data['datetime'] = pandas.to_datetime(data['time'], unit='s', utc=True)
        data = self.aggPerFill(data)
        return data

    def fills(self) -> pandas.DataFrame:
        self.query = '/afs/cern.ch/user/a/adelanno/.local/bin/brilcalc lumi --output-style csv --tssec '
        self.selection()
        self.filtering()
        logging.debug(self.query)
        response = subprocess.run(shlex.split(self.query), capture_output=True, text=True)
        if response.stderr:
            sys.exit(logging.error(response.stderr))
        if not response.returncode:
            rows = response.stdout.splitlines()
            normtag = rows[0].lstrip('#')
            names = rows[1].lstrip('#').split(',')
            summary = dict(zip(rows[-2].lstrip('#').split(','), rows[-1].lstrip('#').split(',')))
            logging.debug(normtag)
            logging.debug(summary)
            data = pandas.read_csv(io.StringIO(response.stdout), names=names, comment='#')
            return self.postProcess(data)
        else:
            return pandas.DataFrame()

    def selection(self):
        if self.year:
            self.query += f"--begin '01/01/{str(self.year)[2:]} 00:00:00' "
            self.query += f"--end '12/31/{str(self.year)[2:]} 23:59:59' "
        if self.begin:
            self.query += f"--begin '{self.parseDatetime(self.begin)}' "
        if self.end:
            self.query += f"--end '{self.parseDatetime(self.end)}' "

    def filtering(self):
        if self.stable_beams:
            self.query += "-b 'stable beams' "
        if self.unit:
            self.query += f"-u {self.unit} "


@dataclasses.dataclass
class BRILFillValidation:
    pass


@dataclasses.dataclass
class LPC:
    '''https://lpc.web.cern.ch/annotatedFillTable.html'''
    year: int = None

    def __post_init__(self):
        self.csvFile = pathlib.Path(f'{path}/{self.year}_{self.__class__.__name__}.csv')

    def fills(self) -> pandas.DataFrame:
        url = f'https://lpc.web.cern.ch/cgi-bin/fillTableReader.py?action=load&year={self.year}'
        logging.debug(url)
        data = pandas.read_json(url).get('data')
        data = pandas.json_normalize(data)
        return data.rename(columns={'ta': 'turn-around-time_statistics', 'fl': 'fill-length_statistics'})


@dataclasses.dataclass
class OMS:
    '''https://cmsoms.cern.ch/agg/api/v1/version/endpoints'''
    stable_beams: bool = None
    year: int = None
    start_time: str = None
    end_time: str = None
    limit: int = None
    offset: int = 0
    url:str =  f'{endpoint}/agg/api/v1'
    verify_SSL: bool = False

    def __post_init__(self):
        self.csvFile = pathlib.Path(f'{path}/{self.year}_{self.__class__.__name__}.csv')

    @staticmethod
    def cookies(cookie_file: pathlib.Path) -> typing.Dict[str,str]:
        '''[Using cookies.txt file with Python Requests](https://stackoverflow.com/a/54659484/13019084)'''
        cookies = pandas.read_csv(cookie_file, sep='\t', header=None, skiprows=[0,1,2,3], usecols=[5,6])
        return dict(zip(cookies[5], cookies[6]))

    def filtering(self):
        if self.year:
            self.start_time = f'{self.year}-01-01'
            self.end_time = f'{self.year}-12-31'
        if self.start_time:
            self.url += f'filter[start_time][GE]={self.start_time}&'
        if self.end_time:
            self.url += f'filter[end_time][LE]={self.end_time}&'
        if self.stable_beams:
            self.url += f'filter[stable_beams][EQ]={self.stable_beams}&'

    def pagination(self):
        if self.limit:
            self.url += f'page[limit]={self.limit}&'
        if self.offset:
            self.url += f'page[offset]={self.offset}&'
    
    def fills(self) -> pandas.DataFrame:
        self.url += f'/fills/csv?'
        self.filtering()
        self.pagination()
        logging.debug(self.url)
        response = requests.get(url=self.url, cookies=self.cookies(cookie_file=cookie_file), verify=self.verify_SSL)
        if 'cern single sign-on' in response.text.lower():
            sys.exit(logging.error('authentication failed'))
        if response.ok:
            return pandas.read_csv(io.StringIO(response.text))
        else:
            return pandas.DataFrame()


@dataclasses.dataclass
class TIMBER:
    '''pytimber'''

    def emittanceScans(self):
        pass


@dataclasses.dataclass
class Fills:
    limit: int = 10000
    currentYear: int = pandas.Timestamp.now().year
    sleep: int = 1

    @staticmethod
    def downloadCSV(classInstance: typing.Union[OMS, LPC], overwrite: bool = False):
        if classInstance.csvFile.exists() and not overwrite:
            return
        else:
            fills = classInstance.fills()
            if not fills.empty:
                fills.to_csv(classInstance.csvFile)

    @staticmethod
    def readCSV(filepath: pathlib.Path, parse_dates: bool = False) -> pandas.DataFrame:
        def parseDate(x): return pandas.to_datetime(x, format='%Y-%m-%dT%H:%M:%SZ')
        if not filepath.exists():
            return pandas.DataFrame()
        timestampColumns = ['start_time','start_stable_beam','end_stable_beam','end_time']
        if parse_dates:
            fills = pandas.read_csv(filepath, index_col=[0], parse_dates=timestampColumns, date_parser=parseDate)
        else:
            fills = pandas.read_csv(filepath, index_col=[0])
        return fills

    def update(self) -> pandas.DataFrame:
        for year in range(2008, self.currentYear):
            self.downloadCSV(BRILCALC(year=year))
            self.downloadCSV(LPC(year=year))
            self.downloadCSV(OMS(year=year, limit=self.limit))
            logging.debug(f'sleep for {self.sleep} seconds...')
            time.sleep(self.sleep)
        self.downloadCSV(BRILCALC(year=self.currentYear), overwrite=True)
        self.downloadCSV(LPC(year=self.currentYear), overwrite=True)
        self.downloadCSV(OMS(year=self.currentYear, limit=self.limit), overwrite=True)

    def fills(self):
        brilcalc = pandas.concat([self.readCSV(filepath = BRILCALC(year=year).csvFile) for year in range(2008, self.currentYear+1)], axis=0).add_prefix('brilcalc_')
        lpc = pandas.concat([self.readCSV(filepath = LPC(year=year).csvFile) for year in range(2008, self.currentYear+1)], axis=0).add_prefix('lpc_')
        oms = pandas.concat([self.readCSV(filepath = OMS(year=year).csvFile) for year in range(2008, self.currentYear+1)], axis=0).add_prefix('oms_')
        fills = pandas.merge(brilcalc, lpc, how='left', left_on='brilcalc_fill', right_on='lpc_fillno')
        fills = pandas.merge(oms, fills, how='left', left_on='oms_fill_number', right_on='brilcalc_fill')
        fills.to_csv(pathlib.Path(f'{path}/../fills.csv'), index=False)

    def updateOMS(self) -> pandas.DataFrame:
        past = pandas.concat([self.readCSV(filepath = OMS(year=year).csvFile) for year in range(2008, self.currentYear)], axis=0)
        current = OMS(year=self.currentYear, limit=self.limit).fill_summary()
        if not current.empty:
            current.to_csv()

if __name__ == '__main__':
    # _ = SSO(verbose=True).auth()
    Fills().update()
    Fills().fills()
