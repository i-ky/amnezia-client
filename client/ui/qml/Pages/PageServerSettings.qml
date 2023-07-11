import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PageEnum 1.0
import "./"
import "../Controls"
import "../Config"

PageBase {
    id: root
    page: PageEnum.ServerSettings
    logic: ServerSettingsLogic

    enabled: ServerSettingsLogic.pageEnabled

    BackButton {
        id: back
    }

    Caption {
        id: caption
        text: qsTr("Server settings")
        anchors.horizontalCenter: parent.horizontalCenter
    }

    FlickableType {
        id: fl
        anchors.top: caption.bottom
        anchors.bottom: logo.top
        contentHeight: content.height

        ColumnLayout {
            id: content
            enabled: logic.pageEnabled
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: 15

            LabelType {
                Layout.fillWidth: true
                font.pixelSize: 20
                horizontalAlignment: Text.AlignHCenter
                text: ServerSettingsLogic.labelCurrentVpnProtocolText
            }

            TextFieldType {
                Layout.fillWidth: true
                font.pixelSize: 20
                horizontalAlignment: Text.AlignHCenter
                text: ServerSettingsLogic.labelServerText
                readOnly: true
                background: Item {}
            }

            LabelType {
                Layout.fillWidth: true
                text: ServerSettingsLogic.labelWaitInfoText
                visible: ServerSettingsLogic.labelWaitInfoVisible
            }
            TextFieldType {
                Layout.fillWidth: true
                text: ServerSettingsLogic.lineEditDescriptionText
                onEditingFinished: {
                    ServerSettingsLogic.lineEditDescriptionText = text
                    ServerSettingsLogic.onLineEditDescriptionEditingFinished()
                }
            }

            BlueButtonType {
                text: qsTr("Protocols and Services")
                Layout.topMargin: 20
                Layout.fillWidth: true
                onClicked: {
                    UiLogic.goToPage(PageEnum.ServerContainers)
                }
            }

            BlueButtonType {
                Layout.fillWidth: true
                Layout.topMargin: 10
                text: qsTr("Share Server (FULL ACCESS)")
                visible: ServerSettingsLogic.pushButtonShareFullVisible
                onClicked: {
                    ServerSettingsLogic.onPushButtonShareFullClicked()
                }
            }

            BlueButtonType {
                Layout.fillWidth: true
                Layout.topMargin: 10
                text: qsTr("Advanced server settings")
                onClicked: {
                    UiLogic.goToPage(PageEnum.AdvancedServerSettings)
                }
            }

            BlueButtonType {
                Layout.fillWidth: true
                Layout.topMargin: 60
                text: ServerSettingsLogic.pushButtonClearClientCacheText
                visible: ServerSettingsLogic.pushButtonClearClientCacheVisible
                onClicked: {
                    ServerSettingsLogic.onPushButtonClearClientCacheClicked()
                }
            }

            BlueButtonType {
                Layout.fillWidth: true
                Layout.topMargin: 10
                text: qsTr("Forget this server")
                onClicked: {
                    if (ServerSettingsLogic.isCurrentServerHasCredentials()) {
                        popupForgetServer.questionText = "Attention! This action will not remove any data from the server, it will just remove server from the list. Continue?"
                    }
                    else {
                        popupForgetServer.questionText = "Remove server from the list?"
                    }
                    popupForgetServer.open()
                }
            }

            PopupWithQuestion {
                id: popupForgetServer
                yesFunc: function() {
                    ServerSettingsLogic.onPushButtonForgetServer()
                    close()
                }
                noFunc: function() {
                    close()
                }
            }
        }
    }

    Logo {
        id : logo
        anchors.bottom: parent.bottom
    }
}
