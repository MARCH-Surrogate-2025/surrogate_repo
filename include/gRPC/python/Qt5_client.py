import sys
from PyQt5.QtWidgets import QApplication, QWidget, QMainWindow, QDesktopWidget
from PyQt5.QtWidgets import QSlider, QDial, QPushButton, QLabel
from PyQt5.QtCore import QCoreApplication
from PyQt5.QtCore import QDate, QTime, QDateTime, Qt
from PyQt5.QtGui import QFont

from custom_client import CustomClient

class MyApp(QMainWindow):

    def __init__(self):
        super().__init__()

        self._env_width = 1920
        self._env_height = 1080
        self._window_width = self._env_width//2
        self._window_height = self._env_height // 2

        self._default_font = QFont('Sans Serif', 9)

        self._custom_client = CustomClient(connect_ip='192.168.0.52', connect_port=50051)

        self.date = QDateTime.currentDateTime()
        self.initUI()

    def initUI(self):
        # Set window properties
        self.setWindowTitle('Surrogate')
        self.setGeometry(0, 0, self._window_width, self._window_height) # (w_lu_corner, h_lu_corner, w_window, h_window)
        self.center()

        # Status bar
        self.statusBar().showMessage(self.date.toString(Qt.DefaultLocaleShortDate))

        # QtItems
        self._Qt_items = []

        # Wheel velocity (slider)
        slider_w = 30
        slider_h = self._window_height//2 + 50
        self._slider_wheel_velocity = QSlider(Qt.Horizontal, self)
        self._slider_wheel_velocity.move(slider_w+200, slider_h)
        self._slider_wheel_velocity.setFixedSize(160, 20)
        self._slider_wheel_velocity.setRange(-700, 700)
        self._slider_wheel_velocity.setSingleStep(25)
        self._slider_wheel_velocity.setTickInterval(100)
        self._slider_wheel_velocity.setTickPosition(QSlider.TicksBelow)
        self._Qt_items.append(self._slider_wheel_velocity)

        self._label_wheel_velocity_legend = QLabel('Wheel velocity (rpm)', self)
        self._label_wheel_velocity_legend.setAlignment(Qt.AlignCenter)
        self._label_wheel_velocity_legend.move(slider_w, slider_h)
        self._label_wheel_velocity_legend.setFixedSize(160, 20)
        self._label_wheel_velocity_legend.setFont(self._default_font)
        self._Qt_items.append(self._label_wheel_velocity_legend)

        self._label_wheel_velocity_command = QLabel('{:d}'.format(0), self)
        self._label_wheel_velocity_command.setAlignment(Qt.AlignCenter)
        self._label_wheel_velocity_command.move(slider_w + 200, slider_h + 20)
        self._label_wheel_velocity_command.setFixedSize(160, 20)
        self._label_wheel_velocity_command.setFont(self._default_font)
        self._Qt_items.append(self._label_wheel_velocity_command)

        # Steering angle (slider)
        slider_w = 30
        slider_h = self._window_height // 2 + 100
        self._slider_steering_position = QSlider(Qt.Horizontal, self)
        self._slider_steering_position.move(slider_w + 200, slider_h)
        self._slider_steering_position.setFixedSize(160, 20)
        self._slider_steering_position.setRange(-90, 90)
        self._slider_steering_position.setSingleStep(15)
        self._slider_steering_position.setTickInterval(15)
        self._slider_steering_position.setTickPosition(QSlider.TicksBelow)
        self._Qt_items.append(self._slider_steering_position)

        self._label_steering_position_legend = QLabel('Steering position (deg)', self)
        self._label_steering_position_legend.setAlignment(Qt.AlignCenter)
        self._label_steering_position_legend.move(slider_w, slider_h)
        self._label_steering_position_legend.setFixedSize(160, 20)
        self._label_steering_position_legend.setFont(self._default_font)
        self._Qt_items.append(self._label_steering_position_legend)

        self._label_steering_position_command = QLabel('{:d}'.format(0), self)
        self._label_steering_position_command.setAlignment(Qt.AlignCenter)
        self._label_steering_position_command.move(slider_w + 200, slider_h + 20)
        self._label_steering_position_command.setFixedSize(160, 20)
        self._label_steering_position_command.setFont(self._default_font)
        self._Qt_items.append(self._label_steering_position_command)

        # mobile state reset (button)
        slider_w = 420
        slider_h = self._window_height // 2 + 75
        self._mobile_reset_btn = QPushButton('Reset', self)
        self._mobile_reset_btn.move(slider_w, slider_h-15)
        self._mobile_reset_btn.resize(80, 50)
        self._mobile_reset_btn.setFont(self._default_font)
        self._Qt_items.append(self._mobile_reset_btn)

        # System emergency stop button
        self._SS_EMSTOP_btn = QPushButton('Emergency\nStop', self)
        self._SS_EMSTOP_btn.move( 50, 50)
        self._SS_EMSTOP_btn.resize(80, 48)
        self._SS_EMSTOP_btn.setFont(self._default_font)
        self._SS_EMSTOP_btn.setEnabled(True)
        self._Qt_items.append(self._SS_EMSTOP_btn)

        # System reset button
        self._SS_RESET_btn = QPushButton('Reset', self)
        self._SS_RESET_btn.move(140, 50)
        self._SS_RESET_btn.resize(80, 48)
        self._SS_RESET_btn.setFont(self._default_font)
        self._SS_RESET_btn.setEnabled(False)
        self._Qt_items.append(self._SS_RESET_btn)

        # close button
        self._window_close_btn = QPushButton('Close', self)
        self._window_close_btn.move(self._window_width - 90, self._window_height - 34)
        self._window_close_btn.resize(80, 24)
        self._window_close_btn.setFont(self._default_font)
        self._Qt_items.append(self._window_close_btn)

        # Set events
        self._mobile_reset_btn.clicked.connect(self.reset_mobile_states)
        self._window_close_btn.clicked.connect(QCoreApplication.instance().quit)

        self._slider_wheel_velocity.valueChanged.connect(self.update_mobile_control)
        self._slider_steering_position.valueChanged.connect(self.update_mobile_control)

        self._SS_EMSTOP_btn.clicked.connect(self.clicked_SS_EMSTOP_button)
        self._SS_RESET_btn.clicked.connect(self.clicked_SS_RESET_button)
        # Display

        # for qt_item in self._Qt_items:
        #     qt_item.setStyleSheet("border: 1px solid black;")

        self.show()

    def reset_mobile_states(self):
        self._slider_wheel_velocity.setValue(0)
        self._slider_steering_position.setValue(0)

    def clicked_SS_EMSTOP_button(self):
        self._SS_RESET_btn.setEnabled(True)
        self._SS_EMSTOP_btn.setEnabled(False)

        self._custom_client.SystemEmergencyStop()

    def clicked_SS_RESET_button(self):
        self._SS_EMSTOP_btn.setEnabled(True)
        self._SS_RESET_btn.setEnabled(False)

        self._custom_client.SystemReset()

    def update_mobile_control(self, value):
        wheel_vel = self._slider_wheel_velocity.value()
        steering_pos = self._slider_steering_position.value()

        self._label_wheel_velocity_command.setText('{:d}'.format(wheel_vel))
        self._label_steering_position_command.setText('{:d}'.format(steering_pos))

        self._custom_client.SetMobileState(wheel_vel, steering_pos)

    def center(self):
        qr = self.frameGeometry()
        cp = QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())


if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = MyApp()
    sys.exit(app.exec_())