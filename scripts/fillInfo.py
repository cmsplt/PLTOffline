#!/cvmfs/cms-bril.cern.ch/brilconda3/bin/python3

import dataclasses
import getpass
import io
import json
import logging
import os
import pathlib
import socket
import subprocess
import sys
import time
import typing
import urllib3

import requests
import pandas

logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

if not 'lxplus' in socket.gethostname():
    sys.exit(logging.error('please run this script from lxplus'))

cookie_file: pathlib.Path = pathlib.Path(f'cmsoms.cookie') # pathlib.Path(f'{pathlib.Path.home()}/private/cmsoms.cookie')
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
            logging.info(sso.stdout)
        if sso.stderr:
            logging.error(sso.stderr)
        return sso.returncode

    # def cernSSO(verify: bool = False):
    #     import lxml.html
    #     import requests
    #     import requests_gssapi
    #     AUTH_HOSTNAME = 'auth.cern.ch'
    #     AUTH_REALM = 'cern'
    #     session = requests.Session()
    #     login_page = session.get(url=url, verify=verify)
    #     tree = lxml.html.fromstring(login_page.text)
    #     kerberos_path = tree.xpath('//a[@id="social-kerberos"]')[0].attrib.get('href')
    #     kerberos_redirect = session.get(url=f'https://{AUTH_HOSTNAME}{kerberos_path}')
    #     kerberos_auth = session.get(url=kerberos_redirect.url, auth=requests_gssapi.HTTPSPNEGOAuth(mutual_authentication=requests_gssapi.OPTIONAL), allow_redirects=False)
    #     while kerberos_auth.status_code == 302 and AUTH_HOSTNAME in kerberos_auth.headers.get('Location'):
    #         kerberos_auth = session.get(url=kerberos_auth.headers.get('Location'), allow_redirects=False)


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
    
    def fills(self, start_time:str = None, end_time: str = None, page_offset: int = 0):
        self.url += f'/fills/csv?'
        self.filtering()
        self.pagination()
        logging.info(self.url)
        response = requests.get(url=self.url, cookies=self.cookies(cookie_file=cookie_file), verify=self.verify_SSL)
        if 'cern single sign-on' in response.text.lower():
            sys.exit(logging.error('authentication failed'))
        if response.ok:
            return pandas.read_csv(io.StringIO(response.text))
        else:
            return pandas.DataFrame()


@dataclasses.dataclass
class LPC:
    '''https://lpc.web.cern.ch/annotatedFillTable.html'''
    year: int = None

    def __post_init__(self):
        self.csvFile = pathlib.Path(f'{path}/{self.year}_{self.__class__.__name__}.csv')

    def fills(self):
        url = f'https://lpc.web.cern.ch/cgi-bin/fillTableReader.py?action=load&year={self.year}'
        logging.info(url)
        data = pandas.read_json(url).get('data')
        data = pandas.json_normalize(data)
        return data.rename(columns={'ta': 'turn-around-time_statistics', 'fl': 'fill-length_statistics'})


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
            self.downloadCSV(LPC(year=year))
            self.downloadCSV(OMS(year=year, limit=self.limit))
            logging.info(f'sleep for {self.sleep} seconds...')
            time.sleep(self.sleep)
        self.downloadCSV(LPC(year=self.currentYear), overwrite=True)
        self.downloadCSV(OMS(year=self.currentYear, limit=self.limit), overwrite=True)

    def fills(self):
        oms = pandas.concat([self.readCSV(filepath = OMS(year=year).csvFile) for year in range(2008, self.currentYear+1)], axis=0).add_prefix('oms_')
        lpc = pandas.concat([self.readCSV(filepath = LPC(year=year).csvFile) for year in range(2008, self.currentYear+1)], axis=0).add_prefix('lpc_')
        fills = pandas.merge(oms, lpc, how='left', left_on='oms_fill_number', right_on='lpc_fillno')
        fills.to_csv(pathlib.Path(f'{path}/../fills.csv'))

    def updateOMS(self) -> pandas.DataFrame:
        past = pandas.concat([self.readCSV(filepath = OMS(year=year).csvFile) for year in range(2008, self.currentYear)], axis=0)
        current = OMS(year=self.currentYear, limit=self.limit).fill_summary()
        if not current.empty:
            current.to_csv()

if __name__ == '__main__':
    _ = SSO(verbose=True).auth()
    Fills().update()
    Fills().fills()
