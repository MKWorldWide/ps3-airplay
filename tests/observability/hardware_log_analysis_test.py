import importlib.util
from pathlib import Path
import unittest
spec = importlib.util.spec_from_file_location('audit', Path(__file__).parents[2] / 'scripts/analyze-hardware-log.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)
class AnalysisTest(unittest.TestCase):
    def test_counter_semantics_and_boundaries(self):
        rows = [('APP_START',0,0),('PKG_VERSION_0.2.5',0,0),('RTP_MISSING_BATCH',9,0),
                ('RTP_MISSING_TOTAL',9,0),('RTP_GAP_CONCEALED',40000,0),
                ('RTP_GAP_CONCEALED_TOTAL',3,0),('ALAC_FAIL_TOTAL',1,0),
                ('ALAC_FAIL_TOTAL',1,0),('APP_START',0,0),('LOG_SESSION_CAPPED',5,0)]
        raw = ''.join('2026-01-01T00:00:00Z event=%s value=%d bytes=%d\n'%r for r in rows).encode()
        r=audit.analyze(raw);self.assertEqual(len(r['sessions']),2)
        e=r['sessions'][0]['events'];self.assertEqual(e['RTP_GAP_CONCEALED_TOTAL']['delta_sum'],3)
        self.assertNotIn('delta_sum',e['RTP_GAP_CONCEALED'])
        self.assertNotIn('delta_sum',e['ALAC_FAIL_TOTAL'])
        self.assertEqual(r['sessions'][1]['cap_markers'],1)
    def test_partial_and_malformed(self):
        r=audit.analyze(b'2026-01-01T00:00:00Z event=LOG_WRAPPED value=1 bytes=0\nbad\n')
        self.assertEqual(r['unparsed_lines'],1);self.assertFalse(r['sessions'][0]['has_app_start'])
if __name__=='__main__': unittest.main()
