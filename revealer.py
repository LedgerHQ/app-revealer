from ledgerblue.comm import getDongle
import binascii
import os
import argparse
import sys
import random
import stat
from decimal import Decimal

try:
    import PyQt5
except Exception:
    sys.exit("Error: Could not import PyQt5. On Linux systems, you may try 'sudo apt-get install python3-pyqt5'")

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtPrintSupport import QPrinter


def auto_int(x):
    return int(x, 0)

def get_argparser():
    parser = argparse.ArgumentParser(description="""Read a sequence of command APDUs from a file and send them to the
device. The file must be formatted as hex, with one CAPDU per line.""")
    parser.add_argument("--noiseSeed", help="noise seed")
    parser.add_argument("--apdu", help="Display APDU log", action='store_true')
    parser.add_argument("--targetId", help="The device's target ID (default is Ledger Nano S)", type=auto_int)
    return parser


def make_dir(path, allow_symlink=True):
    """Make directory if it does not yet exist."""
    if not os.path.exists(path):
        if not allow_symlink and os.path.islink(path):
            raise Exception('Dangling link: ' + path)
        os.mkdir(path)
        os.chmod(path, stat.S_IRUSR | stat.S_IWUSR | stat.S_IXUSR)

class Buttons(QHBoxLayout):
    def __init__(self, *buttons):
        QHBoxLayout.__init__(self)
        self.addStretch(1)
        for b in buttons:
            self.addWidget(b)

class Revealer(QWidget):
    def __init__(self):
        QWidget.__init__(self)

        self.size = (159, 97)
        self.version = '0'
        self.f_size = QSize(1014 * 2, 642 * 2)
        self.abstand_h = 21
        self.abstand_v = 34

        self.calibration_noise = int('10' * 128)

        self.calibration_h = 0#self.config.get('calibration_h')
        self.calibration_v = 0#self.config.get('calibration_v')

        self.base_dir = os.getcwd() + "/backups/"
        make_dir(self.base_dir)
        self.setGeometry(210, 210, 320, 210)

        vbox = QVBoxLayout(self)
        vbox.addSpacing(21)
        logo_r = QLabel()
        vbox.addWidget(logo_r)
        logo_r.setPixmap(QPixmap(os.getcwd() + '/revealer.png'))
        logo_r.setAlignment(Qt.AlignCenter)
        vbox.addSpacing(42)
        bget = QPushButton("Get encrypted seed from Ledger device")
        bget.clicked.connect(self.get_cypherseed)
        vbox.addWidget(bget)
        bcalibrate = QPushButton("Calibrate printer")
        bcalibrate.clicked.connect(self.calibration_dialog)
        vbox.addWidget(bcalibrate)

        vbox.addSpacing(11)

        self.setWindowTitle('Revealer')
        self.show()

    def calibration_dialog(self):

        d =  QDialog(self)
        d.setWindowModality(Qt.WindowModal)
        d.setWindowTitle("Revealer - Printer calibration settings")

        d.setMinimumSize(100, 200)

        vbox = QVBoxLayout(d)
        vbox.addWidget(QLabel(''.join(["<br/>", "If you have an old printer, or want optimal precision", "<br/>",
                                       "print the calibration pdf and follow the instructions ", "<br/>", "<br/>",
                                       ])))
        #self.config.get('calibration_h')
        #self.config.get('calibration_v')
        cprint = QPushButton("Open calibration pdf")
        cprint.clicked.connect(self.calibration)
        vbox.addWidget(cprint)

        vbox.addWidget(QLabel('Calibration values:'))
        grid = QGridLayout()
        vbox.addLayout(grid)
        grid.addWidget(QLabel('Right side'), 0, 0)
        horizontal = QLineEdit()
        horizontal.setText(str(self.calibration_h))
        grid.addWidget(horizontal, 0, 1)

        grid.addWidget(QLabel('Bottom'), 1, 0)
        vertical = QLineEdit()
        vertical.setText(str(self.calibration_v))
        grid.addWidget(vertical, 1, 1)

        vbox.addStretch()
        vbox.addSpacing(13)

        closeButton = QPushButton("Close")
        closeButton.clicked.connect(d.close)
        #vbox.addWidget(closeButton)

        OkButton = QPushButton("Ok")
        OkButton.clicked.connect(d.accept)
        #vbox.addWidget(cprint)

        vbox.addLayout(Buttons(closeButton, OkButton))

        if not d.exec_():
            return

        self.calibration_h = int(Decimal(horizontal.text()))
        #self.config.set_key('calibration_h', self.calibration_h)
        self.calibration_v = int(Decimal(vertical.text()))
        #self.config.set_key('calibration_v', self.calibration_v)
        print(self.calibration_h)

    def make_calnoise(self):
        random.seed(self.calibration_noise)
        w = self.size[0]
        h = self.size[1]
        rawnoise = QImage(w, h, QImage.Format_Mono)
        for x in range(w):
            for y in range(h):
                rawnoise.setPixel(x,y,random.randint(0, 1))
        calnoise = self.pixelcode_2x2(rawnoise)
        return calnoise

    def toPdf(self, image):
        #img = QImage(image.width, image.height, QImage.Format_Mono)
        printer = QPrinter()
        printer.setPaperSize(QSizeF(210, 297), QPrinter.Millimeter)
        printer.setResolution(600)
        printer.setOutputFormat(QPrinter.PdfFormat)
        printer.setOutputFileName(self.base_dir + '/cypher.pdf')
        printer.setPageMargins(0, 0, 0, 0, 6)
        painter = QPainter()
        painter.begin(printer)

        delta_h = round(image.width() / 34)
        delta_v = round(image.height() / 21)

        size_h = 2028 + ((int(self.calibration_h) * 2028 / (2028 - (delta_h * 2) + int(self.calibration_h))) / 2)
        size_v = 1284 + ((int(self.calibration_v) * 1284 / (1284 - (delta_v * 2) + int(self.calibration_v))) / 2)

        image = image.scaled(size_h, size_v)

        painter.drawImage(553, 533, image)
        wpath = QPainterPath()
        wpath.addRoundedRect(QRectF(553, 533, size_h, size_v), 19, 19)
        painter.setPen(QPen(Qt.black, 1))
        painter.drawPath(wpath)
        painter.end()

    def pixelcode_2x2(self, img):
        result = QImage(img.width() * 2, img.height() * 2, QImage.Format_Mono)
        for x in range(img.width()):
            for y in range(img.height()):
                c = img.pixel(QPoint(x, y))
                colors = QColor(c).getRgbF()
                if colors[0]:
                    result.setPixel(x * 2 + 1, y * 2 + 1, 1)
                    result.setPixel(x * 2, y * 2 + 1, 0)
                    result.setPixel(x * 2 + 1, y * 2, 0)
                    result.setPixel(x * 2, y * 2, 1)
                else:
                    result.setPixel(x * 2 + 1, y * 2 + 1, 0)
                    result.setPixel(x * 2, y * 2 + 1, 1)
                    result.setPixel(x * 2 + 1, y * 2, 1)
                    result.setPixel(x * 2, y * 2, 0)
        return result

    def overlay_marks(self, img, calibration_sheet=False):
        f_size = QSize(1014 * 2, 642 * 2)
        abstand_v = 34

        border_color = Qt.white
        base_img = QImage(f_size.width(), f_size.height(), QImage.Format_ARGB32)
        base_img.fill(border_color)
        img = QImage(img)

        painter = QPainter()
        painter.begin(base_img)

        total_distance_h = round(base_img.width() / abstand_v)
        dist_v = round(total_distance_h) / 2
        dist_h = round(total_distance_h) / 2

        img = img.scaledToWidth(base_img.width() - (2 * (total_distance_h)))
        painter.drawImage(total_distance_h,
                          total_distance_h,
                          img)

        # frame around image
        pen = QPen(Qt.black, 2)
        painter.setPen(pen)

        # horz
        painter.drawLine(0, total_distance_h, base_img.width(), total_distance_h)
        painter.drawLine(0, base_img.height() - (total_distance_h), base_img.width(),
                         base_img.height() - (total_distance_h))
        # vert
        painter.drawLine(total_distance_h, 0, total_distance_h, base_img.height())
        painter.drawLine(base_img.width() - (total_distance_h), 0, base_img.width() - (total_distance_h), base_img.height())

        # border around img
        border_thick = 6
        Rpath = QPainterPath()
        Rpath.addRect(QRectF((total_distance_h) + (border_thick / 2),
                             (total_distance_h) + (border_thick / 2),
                             base_img.width() - ((total_distance_h) * 2) - ((border_thick) - 1),
                             (base_img.height() - ((total_distance_h)) * 2) - ((border_thick) - 1)))
        pen = QPen(Qt.black, border_thick)
        pen.setJoinStyle(Qt.MiterJoin)

        painter.setPen(pen)
        painter.drawPath(Rpath)

        Bpath = QPainterPath()
        Bpath.addRect(QRectF((total_distance_h), (total_distance_h),
                             base_img.width() - ((total_distance_h) * 2), (base_img.height() - ((total_distance_h)) * 2)))
        pen = QPen(Qt.black, 1)
        painter.setPen(pen)
        painter.drawPath(Bpath)

        pen = QPen(Qt.black, 1)
        painter.setPen(pen)
        painter.drawLine(0, base_img.height() / 2, total_distance_h, base_img.height() / 2)
        painter.drawLine(base_img.width() / 2, 0, base_img.width() / 2, total_distance_h)

        painter.drawLine(base_img.width() - total_distance_h, base_img.height() / 2, base_img.width(),
                         base_img.height() / 2)
        painter.drawLine(base_img.width() / 2, base_img.height(), base_img.width() / 2,
                         base_img.height() - total_distance_h)

        if not calibration_sheet:
            painter.setPen(QPen(Qt.black, 1, Qt.DashDotDotLine))
            painter.drawLine(0, dist_v, base_img.width(), dist_v)
            painter.drawLine(dist_h, 0, dist_h, base_img.height())
            painter.drawLine(0, base_img.height() - dist_v, base_img.width(), base_img.height() - (dist_v))
            painter.drawLine(base_img.width() - (dist_h), 0, base_img.width() - (dist_h), base_img.height())

            painter.drawImage(((total_distance_h-3)), ((total_distance_h)),
                              QImage(os.getcwd() + '/ledger.png').scaledToWidth(2.1 * (total_distance_h), Qt.SmoothTransformation))
        else:
            painter.end()

            cal_img = QImage(f_size.width() + 100, f_size.height() + 100,
                             QImage.Format_ARGB32)
            cal_img.fill(Qt.white)

            cal_painter = QPainter()
            cal_painter.begin(cal_img)
            cal_painter.drawImage(0, 0, base_img)

            # black lines in the middle of border top left only
            cal_painter.setPen(QPen(Qt.black, 1, Qt.DashDotDotLine))
            cal_painter.drawLine(0, dist_v, base_img.width(), dist_v)
            cal_painter.drawLine(dist_h, 0, dist_h, base_img.height())

            pen = QPen(Qt.black, 2, Qt.DashDotDotLine)
            cal_painter.setPen(pen)
            n = 15

            cal_painter.setFont(QFont("DejaVu Sans Mono", 21, QFont.Bold))
            for x in range(-n, n):
                # lines on bottom (vertical calibration)
                cal_painter.drawLine((((base_img.width()) / (n * 2)) * (x)) + (base_img.width() / 2) - 13,
                                     x + 2 + base_img.height() - (dist_v),
                                     (((base_img.width()) / (n * 2)) * (x)) + (base_img.width() / 2) + 13,
                                     x + 2 + base_img.height() - (dist_v))

                num_pos = 9
                if x > 9: num_pos = 17
                if x < 0: num_pos = 20
                if x < -9: num_pos = 27

                cal_painter.drawText((((base_img.width()) / (n * 2)) * (x)) + (base_img.width() / 2) - num_pos,
                                     50 + base_img.height() - (dist_v),
                                     str(x))

                # lines on the right (horizontal calibrations)

                cal_painter.drawLine(x + 2 + (base_img.width() - (dist_h)),
                                     ((base_img.height() / (2 * n)) * (x)) + (base_img.height() / n) + (
                                             base_img.height() / 2) - 13,
                                     x + 2 + (base_img.width() - (dist_h)),
                                     ((base_img.height() / (2 * n)) * (x)) + (base_img.height() / n) + (
                                             base_img.height() / 2) + 13)

                cal_painter.drawText(30 + (base_img.width() - (dist_h)),
                                     ((base_img.height() / (2 * n)) * (x)) + (base_img.height() / 2) + 13, str(x))

            cal_painter.end()
            base_img = cal_img

        return base_img

    def updateColumn(self, img, b, x, refresh = False):
        # print(x)
        white = qRgba(255, 255, 255, 0)
        black = qRgba(0, 0, 0, 255)
        for i in range(img.height()):
            if b[i]==0:
                img.setPixel(x,i,1)
            if b[i]==1:
                img.setPixel(x, i, 0)
        if refresh is True:
            img.save(os.getcwd() + '/column.png')
        return img

    def get_cypherseed(self):
        img = QImage(159,97, QImage.Format_Mono)

        args = get_argparser().parse_args()
        dongle = getDongle(args.apdu)

        for i in range(img.width()):
            data = binascii.unhexlify("80CB00" + "0x{:02x}".format(i)[2:] + "00")
            print(f"=> {data.hex()}")
            result = dongle.exchange(bytearray(data))
            print(f"<= {result.hex()}")
            img = self.updateColumn(img,result,i, False)
            #if args.apdu:
            #    print("<= Clear " + str(result))
        
        print(self.base_dir)
        img.save(self.base_dir+ '/test.png')
        cypher = self.pixelcode_2x2(img)
        cypher.invertPixels()
        cypher = self.overlay_marks(cypher)
        cypher = cypher.scaled(1014, 642)
        cypher.save(self.base_dir+ '/encrypted_backup.png')
        self.toPdf(QImage(cypher))


    def calibration(self):
        img = QImage(self.size[0],self.size[1], QImage.Format_Mono)
        bitmap = QBitmap.fromImage(img, Qt.MonoOnly)
        bitmap.fill(Qt.black)
        calnoise = self.make_calnoise()
        img = self.overlay_marks(calnoise.scaledToHeight(self.f_size.height()),True)
        self.calibration_pdf(img)
        QDesktopServices.openUrl (QUrl.fromLocalFile(os.path.abspath(os.getcwd()+'/calibration'+'.pdf')))
        return img


    def calibration_pdf(self, image):
        printer = QPrinter()
        printer.setPaperSize(QSizeF(210, 297), QPrinter.Millimeter)
        printer.setResolution(600)
        printer.setOutputFormat(QPrinter.PdfFormat)
        printer.setOutputFileName(os.getcwd()+'/calibration'+'.pdf')
        printer.setPageMargins(0,0,0,0,6)

        painter = QPainter()
        painter.begin(printer)
        painter.drawImage(553,533, image)
        font = QFont('Source Sans Pro', 10, QFont.Bold)
        painter.setFont(font)
        painter.drawText(254,277, "Calibration sheet")
        font = QFont('Source Sans Pro', 7, QFont.Bold)
        painter.setFont(font)
        painter.drawText(600,2077, "Instructions:")
        font = QFont('Source Sans Pro', 7, QFont.Normal)
        painter.setFont(font)
        painter.drawText(700, 2177, "1. Place this paper on a flat and well iluminated surface.")
        painter.drawText(700, 2277, "2. Align your Revealer borderlines to the dashed lines on the top and left.")
        painter.drawText(700, 2377, "3. Press slightly the Revealer against the paper and read the numbers that best "
                                      "match on the opposite sides. ")
        painter.drawText(700, 2477, "4. Type the numbers in the software")
        painter.end()



if __name__ == '__main__':

    app = QApplication(sys.argv)
    r = Revealer()
    app.exec_()
