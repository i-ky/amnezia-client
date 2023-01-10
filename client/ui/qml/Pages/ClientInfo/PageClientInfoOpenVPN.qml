import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ProtocolEnum 1.0
import "../"
import "../../Controls"
import "../../Config"

PageClientInfoBase {
    id: root
    protocol: ProtocolEnum.OpenVpn
    BackButton {
        id: back
    }

    Caption {
        id: caption
        text: qsTr("Client Info")
    }

    Flickable {
        id: fl
        width: root.width
        anchors.top: caption.bottom
        anchors.topMargin: 20
        anchors.bottom: root.bottom
        anchors.bottomMargin: 20
        anchors.left: root.left
        anchors.leftMargin: 30
        anchors.right: root.right
        anchors.rightMargin: 30

        contentHeight: content.height
        clip: true

        ColumnLayout {
            id: content
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            LabelType {
                Layout.fillWidth: true
                font.pixelSize: 20
                horizontalAlignment: Text.AlignHCenter
                text: ClientInfoLogic.labelCurrentVpnProtocolText
            }

            LabelType {
                height: 21
                text: qsTr("Client name")
            }

            TextFieldType {
                Layout.fillWidth: true
                Layout.preferredHeight: 31
                text: ClientInfoLogic.lineEditNameAliasText
                onEditingFinished: {
                    ClientInfoLogic.lineEditNameAliasText = text
                    ClientInfoLogic.onLineEditNameAliasEditingFinished()
                }
            }

            LabelType {
                Layout.topMargin: 20
                height: 21
                text: qsTr("Certificate id")
            }

            LabelType {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: ClientInfoLogic.labelOpenVpnCertId
            }

            LabelType {
                Layout.topMargin: 20
                height: 21
                text: qsTr("Certificate")
            }

            TextAreaType {
                Layout.preferredHeight: 200
                Layout.fillWidth: true

                textArea.readOnly: true
                textArea.wrapMode: TextEdit.WrapAnywhere
                textArea.verticalAlignment: Text.AlignTop
                textArea.text: ClientInfoLogic.textAreaOpenVpnCertData
            }

            BlueButtonType {
                Layout.fillWidth: true
                Layout.preferredHeight: 41
                text: qsTr("Revoke Certificate")
                onClicked: {
                    ClientInfoLogic.onRevokeOpenVpnCertificateClicked()
                }
            }
        }
    }
}
